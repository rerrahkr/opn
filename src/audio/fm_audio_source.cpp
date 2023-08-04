// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "fm_audio_source.h"

#include <limits>
#include <numeric>

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
 * @brief Calculate Block and F-Number.
 * @param[in] noteNumber Note number.
 * @return 2 bytes data which contains Block and F-Number.
 */
std::uint16_t calculateFNumberAndBlock(int noteNumber) {
  constexpr int kC4Number{60}, kSemitoneCountInOctave{12};
  const int block = noteNumber / kSemitoneCountInOctave - 1;
  const int noteNumberInOctave = noteNumber % kSemitoneCountInOctave;
  const double baseHz =
      juce::MidiMessage::getMidiNoteInHertz(kC4Number + noteNumberInOctave);
  const auto baseFNum = static_cast<std::uint16_t>(
      std::round(baseHz * 2304. / (kChipClockHz >> 13)));
  return (static_cast<std::uint8_t>(block) << 11) | baseFNum;
}
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

  // Initialize interruption / YM2608 mode
  reservedChanges_.emplace_back(0x29, 0x80);

  setTone();

  triggerReservedMidiMessages();

  outputDataBuffer_.resize(samplesPerBlockExpected);
}

void FmAudioSource::releaseResources() {}

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

bool FmAudioSource::tryMidiMessageReservation(
    const juce::MidiMessage& message) {
  DBG(message.getDescription());

  bool isSuccess{};

  if (message.isNoteOn()) {
    const auto&& assignments = keyboard_.tryNoteOn(Note(message));
    isSuccess = !assignments.empty();

    for (const auto& assignment : assignments) {
      if (assignment.note.isNoteOn()) {
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
    // TODO: pitch bend
  }

  return isSuccess;
}

void FmAudioSource::triggerReservedMidiMessages() {
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

  // Set pitch.
  const std::uint16_t blockAndFNum =
      calculateFNumberAndBlock(assignment.note.noteNumber);

  static const std::uint16_t kFNum1AddressTable[kMaxChannelCount]{
      0xa0, 0xa1, 0xa2, 0x1a0, 0x1a1, 0x1a2};
  const auto fNum1Address = kFNum1AddressTable[assignment.assignId];
  constexpr std::uint16_t kBlockFNum2AddressOffset{4};
  reservedChanges_.emplace_back(
      fNum1Address + kBlockFNum2AddressOffset,
      static_cast<uint8_t>((blockAndFNum >> 8)));  ///< Block and F-Num2
  reservedChanges_.emplace_back(
      fNum1Address,
      static_cast<std::uint8_t>(blockAndFNum & 0x00ff));  ///< F-Num1

  // Set note-on.
  constexpr std::uint8_t kKeyOffMask{0xf0u};
  reservedChanges_.emplace_back(
      0x28, kNoteOnChannelTable[assignment.assignId] | kKeyOffMask);

  return true;
}

bool FmAudioSource::reserveNoteOff(const NoteAssignment& assignment) {
  if (kMaxChannelCount < assignment.assignId) {
    return false;
  }

  reservedChanges_.emplace_back(0x28, kNoteOnChannelTable[assignment.assignId]);

  return true;
}

void FmAudioSource::setTone() {
  struct Tone {
    struct {
      std::uint8_t ar, dr, sr, rr, sl, tl, ks, ml, dt, am;
    } op[4];

    std::uint8_t al, fb;
  } tone{{
             {31, 8, 0, 0, 9, 24, 0, 1, 0, 0},
             {31, 6, 0, 7, 9, 0, 0, 1, 0, 0},
             {31, 2, 0, 0, 4, 29, 0, 0, 0, 0},
             {31, 1, 0, 7, 1, 0, 0, 1, 0, 0},
         },
         4,
         5};

  for (const std::size_t id : keyboard_.usedAssignIds()) {
    if (kMaxChannelCount <= id) {
      // TODO: Fix polyphonic control
      return;
    }

    const auto writeToBindedChannel =
        [this, offset = kAddressOffsetTableForToneSet[id]](
            std::uint16_t address, std::uint8_t data) {
          reservedChanges_.emplace_back(address | offset, data);
        };

    writeToBindedChannel(0xb0, (tone.fb << 3) | tone.al);

    for (size_t n = 0; n < 4; ++n) {
      const auto& op = tone.op[n];
      const auto writeToBindedOperator =
          [&writeToBindedChannel, offset = kOperatorAddressOffsetTable[n]](
              std::uint16_t address, std::uint8_t data) {
            writeToBindedChannel(address | offset, data);
          };

      writeToBindedOperator(0x30, (op.dt << 4) | op.ml);
      writeToBindedOperator(0x40, op.tl);
      writeToBindedOperator(0x50, (op.ks << 6) | op.ar);
      writeToBindedOperator(0x60, op.dr);
      writeToBindedOperator(0x70, op.sr);
      writeToBindedOperator(0x80, (op.sl << 4) | op.rr);
    }

    writeToBindedChannel(0xb4, kPanpotMask);
  }
}
}  // namespace audio
