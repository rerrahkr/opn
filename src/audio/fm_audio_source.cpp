// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "fm_audio_source.h"

#include <limits>
#include <numeric>

#include "pitch_util.h"

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
 * @brief Look-up table of address offsets for setting tone. The index is
 * channel number.
 */
constexpr std::uint16_t kAddressOffsetTableForToneSet[kMaxChannelCount]{
    0x0u, 0x1u, 0x2u, 0x100u, 0x101u, 0x102u};

/**
 * @brief Look-up table of address offset used to write a parameter of
 * operators. The index is operator number.
 */
constexpr std::uint16_t kOperatorAddressOffsetTable[]{
    0u,
    8u,
    4u,
    12u,
};

/// Mask of panpot set at $b4-$b6. This means "we set panpot to center."
constexpr std::uint8_t kPanpotMask{0xf0u};

/**
 * @brief Look-up table used to control note on/off control in the low nibble of
 * $28. The index is channel number.
 */
constexpr std::uint8_t kNoteOnChannelTable[kMaxChannelCount]{
    0b000u, 0b001u, 0b010u, 0b100u, 0b101u, 0b110u};

/// Default polyphony number.
constexpr std::size_t kDefaultPolyphony{kMaxChannelCount};

/**
 * @brief Calculate F-Number of givien frequency.
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
    std::lock_guard<std::mutex> guard(mutex_);

    // Initialize interruption / YM2608 mode
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
bool FmAudioSource::tryReservePitchBendSensitivityChange(int value) {
  if (pitchBendSensitivity_.load() == value) {
    return false;
  }

  pitchBendSensitivity_.store(static_cast<std::uint8_t>(value));

  return reservePitchChange();
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

    // Pitch bend is insensitive on channel.
    pitchBendSensitivity_.store(
        static_cast<std::uint8_t>(rpnMessage.parameterNumber));

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
  std::lock_guard<std::mutex> guard(mutex_);

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
  std::lock_guard<std::mutex> guard(mutex_);
  reservedChanges_.emplace_back(
      0x28u, kNoteOnChannelTable[assignment.assignId] | noteOnMask_);

  return true;
}

bool FmAudioSource::reserveNoteOff(const NoteAssignment& assignment) {
  if (kMaxChannelCount < assignment.assignId) {
    return false;
  }

  std::lock_guard<std::mutex> guard(mutex_);
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

  const int cent = pitch_util::calculateCent(
      assignment.note.noteNumber, pitchBend_, pitchBendSensitivity_.load());
  const std::uint16_t blockAndFNum = calculateFNumberAndBlockFromCent(cent);

  static const std::uint16_t kFNum1AddressTable[kMaxChannelCount]{
      0xa0u, 0xa1u, 0xa2u, 0x1a0u, 0x1a1u, 0x1a2u};
  const auto fNum1Address = kFNum1AddressTable[assignment.assignId];
  constexpr std::uint16_t kBlockFNum2AddressOffset{4};
  std::lock_guard<std::mutex> guard(mutex_);
  reservedChanges_.emplace_back(
      fNum1Address + kBlockFNum2AddressOffset,
      static_cast<uint8_t>((blockAndFNum >> 8)));  ///< Block and F-Num2
  reservedChanges_.emplace_back(
      fNum1Address,
      static_cast<std::uint8_t>(blockAndFNum & 0x00ff));  ///< F-Num1

  return true;
}

void FmAudioSource::reserveUpdatingAllToneParameter() {
  for (const std::size_t id : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= id) {
      // TODO: Fix polyphonic control
      return;
    }

    const auto writeToBindedChannel =
        [this, offset = kAddressOffsetTableForToneSet[id]](
            std::uint16_t address, std::uint8_t data) {
          std::lock_guard<std::mutex> guard(mutex_);
          reservedChanges_.emplace_back(address | offset, data);
        };

    writeToBindedChannel(0xb0u, ((toneParameterState_.fb.value() & 7u) << 3) |
                                    (toneParameterState_.al.value() & 7u));

    for (size_t n = 0; n < std::size(toneParameterState_.op); ++n) {
      const auto& op = toneParameterState_.op[n];
      const auto writeToBindedOperator =
          [&writeToBindedChannel, offset = kOperatorAddressOffsetTable[n]](
              std::uint16_t address, std::uint8_t data) {
            writeToBindedChannel(address | offset, data);
          };

      const std::uint8_t rawDt =
          (op.dt.value() < 0 ? 4u : 0u) |
          (static_cast<std::uint8_t>(std::abs(op.dt.value())) & 3u);
      writeToBindedOperator(0x30u, (rawDt << 4) | (op.ml.value() & 15u));
      writeToBindedOperator(0x40u, op.tl.value() & 127u);
      const std::uint8_t rawAr =
          op.ssgeg.isEnabled ? 31u : (op.ar.value() & 31u);
      writeToBindedOperator(0x50u, ((op.ks.value() & 3u) << 6) | rawAr);
      writeToBindedOperator(0x60u, op.dr.value() & 31u);
      writeToBindedOperator(0x70u, op.sr.value() & 31u);
      writeToBindedOperator(
          0x80u, ((op.sl.value() & 15u) << 4) | (op.rr.value() & 15u));
      writeToBindedOperator(0x90, static_cast<std::uint8_t>(op.ssgeg.shape));
    }

    writeToBindedChannel(
        0xb4u, kPanpotMask | ((toneParameterState_.lfo.ams.value() & 3u) << 4) |
                   (toneParameterState_.lfo.pms.value() & 7u));
  }

  {
    std::lock_guard<std::mutex> guard(mutex_);
    reservedChanges_.emplace_back(
        0x22u, (toneParameterState_.lfo.isEnabled ? 8u : 0u) |
                   (toneParameterState_.lfo.frequency.value() & 7u));
  }

  // Change note-on mask.
  std::uint8_t noteOnMask{};
  for (std::size_t i = 0; i < std::size(toneParameterState_.op); ++i) {
    noteOnMask |=
        (static_cast<std::uint8_t>(toneParameterState_.op[i].isEnabled) << i);
  }
  noteOnMask_.store(noteOnMask << 4);
}
}  // namespace audio
