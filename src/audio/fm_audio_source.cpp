// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "fm_audio_source.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <utility>

#include "../util.h"
#include "pitch_util.h"
#include "register.h"

namespace audio {
namespace {
/**
 * @brief Clock frequency of YM2608.
 * @remark 3993600 * 2 == 975 << 13.
 */
constexpr unsigned int kChipClockHz{3993600 * 2};

/// Maximum number of channel.
constexpr std::size_t kMaxChannelCount{6};

/**
 * @brief Calculate address added channel offset.
 * @param[in] channel Number of channel.
 * @param[in] baseAddress Address where @c channel is 0.
 * @return Address added offset, or @c baseAddress if @c channel is invalid
 * value.
 */
constexpr std::uint16_t addressOfChannel(std::size_t channel,
                                         std::uint16_t baseAddress) noexcept {
  constexpr std::uint16_t kOffset[kMaxChannelCount]{0x0u,   0x1u,   0x2u,
                                                    0x100u, 0x101u, 0x102u};
  return (channel < kMaxChannelCount) ? (baseAddress + kOffset[channel])
                                      : baseAddress;
}

/**
 * @brief Calculate address added operator offset.
 * @param[in] slot Operator (slot) number.
 * @param[in] baseAddress Address where @c slot is 0.
 * @return Address added offset, or @c baseAddress if @c slot is invalid.
 */
constexpr std::uint16_t addressOfOperator(std::size_t slot,
                                          std::uint16_t baseAddress) noexcept {
  constexpr std::uint16_t kOffset[]{0u, 8u, 4u, 12u};
  return (slot < std::size(kOffset)) ? (baseAddress + kOffset[slot])
                                     : baseAddress;
}

/**
 * @brief Convert detune value from signed to unsigned.
 * @param[in] value Signed detune value.
 * @return Unsigned detune value.
 */
inline std::uint8_t convertDetuneAsRegisterValue(
    const parameter::DetuneValue& value) {
  return (value.rawValue() < 0 ? 4u : 0u) |
         static_cast<std::uint8_t>(std::abs(value.rawValue()));
}

/// Mask of panning set at $b4-$b6. This means "we set panning to center."
constexpr std::uint8_t kPanningMask{0xf0u};

/**
 * @brief Look-up table used to control note on/off control in the low nibble of
 * $28. The index is channel number.
 */
constexpr std::uint8_t kNoteOnChannelTable[kMaxChannelCount]{
    0b000u, 0b001u, 0b010u, 0b100u, 0b101u, 0b110u};

/// Default polyphony number.
constexpr std::size_t kDefaultPolyphony{kMaxChannelCount};

/**
 * @brief Calculate F-Number of given frequency.
 * @param[in] hz Frequency.
 * @return F-Number.
 */
auto calculateFNumber(double hz) {
  return static_cast<std::uint16_t>(
      std::round(hz * 2304. / (kChipClockHz >> 13)));
}

/**
 * @brief Calculate Block and F-Number from cent.
 * @param[in] cent Cent from MIDI note number 0.
 * @return 2 bytes data which contains Block and F-Number.
 */
std::uint16_t calculateFNumberAndBlockFromCent(int cent) {
  constexpr int kC4Cent = pitch_util::kC4NoteNumber * pitch_util::kSemitoneCent;
  constexpr int kOctaveCent =
      pitch_util::kSemitoneCountInOctave * pitch_util::kSemitoneCent;
  const int block = cent / kOctaveCent - 1;
  const int centInOctave = cent % kOctaveCent;

  const double baseHz = pitch_util::calculateHzFromCent(kC4Cent + centInOctave);
  const auto baseFNum = calculateFNumber(baseHz);

  return (static_cast<std::uint8_t>(block) << 11) | baseFNum;
}

// /**
//  * @brief Calculate Block and F-Number from MIDI note number.
//  * @param[in] noteNumber Note number.
//  * @return 2 bytes data which contains Block and F-Number.
//  */
// std::uint16_t calculateFNumberAndBlockFromNoteNumber(int noteNumber) {
//   const int block = noteNumber / pitch_util::kSemitoneCountInOctave - 1;
//   const int noteNumberInOctave =
//       noteNumber % pitch_util::kSemitoneCountInOctave;

//   const double baseHz = juce::MidiMessage::getMidiNoteInHertz(
//       pitch_util::kC4NoteNumber + noteNumberInOctave);
//   const auto baseFNum = calculateFNumber(baseHz);

//   return (static_cast<std::uint8_t>(block) << 11) | baseFNum;
// }
}  // namespace

FmAudioSource::FmAudioSource() : keyboard_(kDefaultPolyphony) {
  ym2608_ = std::make_unique<ymfm::ym2608>(interface_);
  ym2608_->set_fidelity(ymfm::opn_fidelity::OPN_FIDELITY_MIN);
}

FmAudioSource ::~FmAudioSource() = default;

double FmAudioSource::synthesisRate() const { return kChipClockHz / 144; }

void FmAudioSource::prepareToPlay(int samplesPerBlockExpected,
                                  double /*sampleRate*/) {
  ym2608_->reset();

  {
    const std::lock_guard guard(reservedChangesMutex_);

    // Initialise interruption / YM2608 mode
    reservedChanges_.emplace_back(0x29u, 0x80u);
  }

  reserveUpdatingAllToneParameter();

  triggerReservedChanges();

  outputDataBuffer_.resize(samplesPerBlockExpected);

  rpnDetector_.reset();
}

void FmAudioSource::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill) {
  if (outputDataBuffer_.size() < bufferToFill.numSamples) {
    outputDataBuffer_.resize(bufferToFill.numSamples);
  }

  ym2608_->generate_fm_adpcm(
      outputDataBuffer_.data(),
      static_cast<std::uint32_t>(bufferToFill.numSamples));

  if (bufferToFill.buffer->getNumChannels() == 1) {
    // Mono
    std::transform(
        outputDataBuffer_.begin(),
        outputDataBuffer_.begin() + bufferToFill.numSamples,
        bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample),
        [](const ymfm::ym2608::output_data& output) {
          return std::midpoint(static_cast<float>(output.data[0]),
                               static_cast<float>(output.data[1])) /
                 (std::numeric_limits<std::int16_t>::max());
        });
  } else {
    // Stereo
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch) {
      std::transform(
          outputDataBuffer_.begin(),
          outputDataBuffer_.begin() + bufferToFill.numSamples,
          bufferToFill.buffer->getWritePointer(ch, bufferToFill.startSample),
          [ch](const ymfm::ym2608::output_data& output) {
            const auto a = output.data[ch];
            return static_cast<float>(a) /
                   (std::numeric_limits<std::int16_t>::max());
          });
    }
  }
}

// [Changes] -------------------------------------------------------------------
bool FmAudioSource::tryReserveParameterChange(
    const parameter::PitchBendSensitivityValue& value) {
  if (std::lock_guard guard(parameterMutex_);
      std::exchange(pitchBendSensitivity_, value) == value) {
    return false;
  }

  return reservePitchChange();
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::FeedbackValue& value) {
  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  if (std::exchange(toneParameterState_.fb, value) == value) {
    return false;
  }

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(
        addressOfChannel(channel, 0xb0u),
        (value.rawValue() << 3) | toneParameterState_.al.rawValue());
  }
  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::AlgorithmValue& value) {
  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  if (std::exchange(toneParameterState_.al, value) == value) {
    return false;
  }

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(
        addressOfChannel(channel, 0xb0u),
        (toneParameterState_.fb.rawValue() << 3) | value.rawValue());
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::OperatorEnabledValue>&
        slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.isEnabled, value) == value) {
    return false;
  }

  const std::uint8_t mask = 1u << (slot + 4u);
  if (value) {
    noteOnMask_.fetch_or(mask);
  } else {
    noteOnMask_.fetch_and(~mask);
  }

  for ([[maybe_unused]] const auto& [channel, _] : keyboard_.noteOns()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(
        0x28u, kNoteOnChannelTable[channel] | noteOnMask_.load());
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::AttackRateValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.ar, value) == value) {
    return false;
  }

  if (slotParameters.ssgeg.isEnabled) {
    // No need to change register.
    return true;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x50u);
  const std::uint8_t registerValue =
      (slotParameters.ks.rawValue() << 6) | value.rawValue();

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  registerValue);
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::DecayRateValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.dr, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x60u);

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  value.rawValue());
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::SustainRateValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.sr, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x70u);

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  value.rawValue());
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::ReleaseRateValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.rr, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x80u);
  const std::uint8_t registerValue =
      (slotParameters.sl.rawValue() << 4) | value.rawValue();

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  registerValue);
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::SustainLevelValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.sl, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x80u);
  const std::uint8_t registerValue =
      (value.rawValue() << 4) | slotParameters.rr.rawValue();

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  registerValue);
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::TotalLevelValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.tl, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x40u);

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  value.rawValue());
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::KeyScaleValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.ks, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x50u);
  const std::uint8_t rawAr = slotParameters.ssgeg.isEnabled
                                 ? parameter::AttackRateValue::kMaximum
                                 : slotParameters.ar.rawValue();
  const std::uint8_t registerValue = (value.rawValue() << 6) | rawAr;

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  registerValue);
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::MultipleValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.ml, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x30u);
  const std::uint8_t rawDt = convertDetuneAsRegisterValue(slotParameters.dt);
  const std::uint8_t registerValue = (rawDt << 4) | value.rawValue();

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  registerValue);
  }

  return true;
}

bool FmAudioSource::tryReserveParameterChange(
    const parameter::SlotAndValue<parameter::DetuneValue>& slotAndValue) {
  const auto slot = slotAndValue.slot.rawValue();
  const auto& value = slotAndValue.value;

  const std::scoped_lock guard{parameterMutex_, reservedChangesMutex_};

  auto& slotParameters = toneParameterState_.slot[slot];

  if (std::exchange(slotParameters.dt, value) == value) {
    return false;
  }

  const std::uint16_t address = addressOfOperator(slot, 0x30u);
  const std::uint8_t rawDt = convertDetuneAsRegisterValue(value);
  const std::uint8_t registerValue =
      (rawDt << 4) | slotParameters.ml.rawValue();

  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      continue;
    }

    reservedChanges_.emplace_back(addressOfChannel(channel, address),
                                  registerValue);
  }

  return true;
}

bool FmAudioSource::tryReserveChangeFromMidiMessage(
    const juce::MidiMessage& message) {
  DBG(message.getDescription());

  if (message.isController()) {
    // Check pitch bend sensitivity.
    const auto parseResult = rpnDetector_.tryParse(
        message.getChannel(), message.getControllerNumber(),
        message.getControllerValue());
    if (!parseResult.has_value()) {
      return false;
    }

    const auto rpnMessage = parseResult.value();
    if (rpnMessage.isNRPN || rpnMessage.parameterNumber != 0) {
      return false;
    }

    {
      // Pitch bend is insensitive on channel.
      std::lock_guard pbsGuard(parameterMutex_);
      pitchBendSensitivity_ =
          parameter::PitchBendSensitivityValue{rpnMessage.parameterNumber};
    }

    rpnDetector_.reset();

    return reservePitchChange();
  }

  rpnDetector_.reset();

  bool isSuccess{};

  if (message.isNoteOn()) {
    const auto&& assignments = keyboard_.tryNoteOn(Note(message));
    isSuccess = !assignments.empty();

    for (const auto& assignment : assignments) {
      if (assignment.note.isNoteOn()) {
        isSuccess &= reservePitchChange(assignment);
        isSuccess &= reserveNoteOn(assignment);
      } else {
        isSuccess &= reserveNoteOff(assignment);
      }
    }
  } else if (message.isNoteOff()) {
    const auto&& assignment = keyboard_.tryNoteOff(Note(message));
    isSuccess =
        assignment.has_value() ? reserveNoteOff(assignment.value()) : false;
  } else if (message.isPitchWheel()) {
    // Pitch bend is insensitive on channel.
    pitchBend_ = message.getPitchWheelValue() + pitch_util::kMinPitchBend;
    isSuccess = reservePitchChange();
  }

  return isSuccess;
}

void FmAudioSource::triggerReservedChanges() {
  const std::lock_guard guard(reservedChangesMutex_);

  for (const auto& change : reservedChanges_) {
    if (change.pinA1) {
      ym2608_->write_address_hi(change.address);
      ym2608_->write_data_hi(change.data);
    } else {
      ym2608_->write_address(change.address);
      ym2608_->write_data(change.data);
    }
  }
}

bool FmAudioSource::reserveNoteOn(const NoteAssignment& assignment) {
  if (kMaxChannelCount < assignment.assignId) {
    return false;
  }

  // Set note-on.
  const std::lock_guard guard(reservedChangesMutex_);
  reservedChanges_.emplace_back(
      0x28u, kNoteOnChannelTable[assignment.assignId] | noteOnMask_.load());

  return true;
}

bool FmAudioSource::reserveNoteOff(const NoteAssignment& assignment) {
  if (kMaxChannelCount < assignment.assignId) {
    return false;
  }

  const std::lock_guard guard(reservedChangesMutex_);
  reservedChanges_.emplace_back(0x28u,
                                kNoteOnChannelTable[assignment.assignId]);

  return true;
}

bool FmAudioSource::reservePitchChange() {
  bool isSuccess{true};
  for (const auto& assignment : keyboard_.noteOns()) {
    isSuccess &= reservePitchChange(assignment);
  }
  return isSuccess;
}

bool FmAudioSource::reservePitchChange(const NoteAssignment& assignment) {
  if (kMaxChannelCount < assignment.assignId) {
    return false;
  }
  const auto pbs = [&] {
    std::lock_guard guard(parameterMutex_);
    return pitchBendSensitivity_.rawValue();
  }();
  const int cent =
      pitch_util::calculateCent(assignment.note.noteNumber, pitchBend_, pbs);
  const std::uint16_t blockAndFNum = calculateFNumberAndBlockFromCent(cent);

  static const std::uint16_t kFNum1AddressTable[kMaxChannelCount]{
      0xa0u, 0xa1u, 0xa2u, 0x1a0u, 0x1a1u, 0x1a2u};
  const auto fNum1Address = kFNum1AddressTable[assignment.assignId];
  constexpr std::uint16_t kBlockFNum2AddressOffset{4};
  const std::lock_guard guard(reservedChangesMutex_);
  reservedChanges_.emplace_back(
      fNum1Address + kBlockFNum2AddressOffset,
      static_cast<uint8_t>((blockAndFNum >> 8)));  ///< Block and F-Num2
  reservedChanges_.emplace_back(
      fNum1Address,
      static_cast<std::uint8_t>(blockAndFNum & 0x00ff));  ///< F-Num1

  return true;
}

void FmAudioSource::reserveUpdatingAllToneParameter() {
  for (const std::size_t channel : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= channel) {
      // TODO: Fix polyphonic control
      return;
    }

    const auto writeToBoundChannel = [&](std::uint16_t address,
                                         std::uint8_t data) {
      const std::lock_guard guard(reservedChangesMutex_);
      reservedChanges_.emplace_back(addressOfChannel(channel, address), data);
    };

    writeToBoundChannel(0xb0u, (toneParameterState_.fb.rawValue() << 3) |
                                   toneParameterState_.al.rawValue());

    for (std::size_t n = 0; n < audio::kSlotCount; ++n) {
      const auto& op = toneParameterState_.slot[n];
      const auto writeToBoundOperator = [&](std::uint16_t address,
                                            std::uint8_t data) {
        writeToBoundChannel(addressOfOperator(n, address), data);
      };

      const std::uint8_t rawDt = convertDetuneAsRegisterValue(op.dt);
      writeToBoundOperator(0x30u, (rawDt << 4) | op.ml.rawValue());
      writeToBoundOperator(0x40u, op.tl.rawValue());
      const std::uint8_t rawAr = op.ssgeg.isEnabled
                                     ? parameter::AttackRateValue::kMaximum
                                     : op.ar.rawValue();
      writeToBoundOperator(0x50u, (op.ks.rawValue() << 6) | rawAr);
      writeToBoundOperator(0x60u, op.dr.rawValue());
      writeToBoundOperator(0x70u, op.sr.rawValue());
      writeToBoundOperator(0x80u, (op.sl.rawValue() << 4) | op.rr.rawValue());
      writeToBoundOperator(0x90, util::to_underlying(op.ssgeg.shape));
    }

    writeToBoundChannel(
        0xb4u, kPanningMask | (toneParameterState_.lfo.ams.rawValue() << 4) |
                   toneParameterState_.lfo.pms.rawValue());
  }

  {
    const std::lock_guard guard(reservedChangesMutex_);
    reservedChanges_.emplace_back(
        0x22u, (toneParameterState_.lfo.isEnabled ? 8u : 0u) |
                   toneParameterState_.lfo.frequency.rawValue());
  }

  // Change note-on mask.
  std::uint8_t noteOnMask{};
  for (std::size_t i = 0; i < audio::kSlotCount; ++i) {
    noteOnMask |= (static_cast<std::uint8_t>(
                       toneParameterState_.slot[i].isEnabled.rawValue())
                   << i);
  }
  noteOnMask_.store(noteOnMask << 4);
}
}  // namespace audio
