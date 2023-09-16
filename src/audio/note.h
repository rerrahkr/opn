// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <cstdint>

namespace audio {
/**
 * @brief Note object.
 */
struct Note {
  int channel;            ///< Channel.
  int noteNumber;         ///< Note number.
  std::uint8_t velocity;  ///< Velocity.

  /**
   * @brief Constructor.
   * @param[in] channel Channel.
   * @param[in] noteNumber Note number.
   * @param[in] velocity Velocity. If this value is zero, it is handled as
   * note-off.
   */
  Note(int channel, int noteNumber, std::uint8_t velocity) noexcept
      : channel(channel), noteNumber(noteNumber), velocity(velocity) {}

  /**
   * @brief Construct from MIDI message.
   * @param[in] message MIDI message.
   */
  explicit Note(const juce::MidiMessage& message)
      : channel(message.getChannel()),
        noteNumber(message.getNoteNumber()),
        velocity(message.isNoteOn() ? message.getVelocity() : 0u) {}

  /**
   * @brief Generatenote on.
   * @param[in] channel Channel.
   * @param[in] noteNumber Note number.
   * @param[in] velocity Velocity.
   * @return Note-on object.
   */
  static auto noteOn(int channel, int noteNumber, std::uint8_t velocity) {
    return Note(channel, noteNumber, velocity);
  }

  /**
   * @brief Generate note off.
   * @param[in] channel Channel.
   * @param[in] noteNumber Note number.
   * @return Note-off object
   */
  static auto noteOff(int channel, int noteNumber) {
    return Note(channel, noteNumber, 0u);
  }

  /**
   * @brief Check whether this note is note-on or off.
   * @return @c true if note-on, otherwise @c false.
   */
  bool isNoteOn() const noexcept { return velocity != 0u; }
};
}  // namespace audio
