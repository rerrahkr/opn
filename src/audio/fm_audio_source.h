// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>
#include <ymfm_opn.h>

#include <atomic>
#include <memory>
#include <vector>

#include "keyboard.h"

namespace audio {
/**
 * @brief Audio source class for FM part.
 */
class FmAudioSource : public juce::AudioSource {
 public:
  /**
   * @brief Constructor.
   */
  FmAudioSource();

  /**
   * @brief Destructor.
   */
  ~FmAudioSource() override;

  /**
   * @brief Get synthesis rate of FM part.
   * @return FM synthesis rate.
   */
  double synthesisRate() const;

  /**
   * @brief Prepare audio source to play.
   * @details Reset emulation.
   */
  void prepareToPlay(int samplesPerBlockExpected,
                     double /*sampleRate*/) override;

  /**
   * @brief Release resources.
   */
  void releaseResources() override;

  /**
   * @brief Fill samples to a buffer in given channel info.
   * @param[in] bufferToFill Channel info which has a buffer to be stored
   * samples.
   */
  void getNextAudioBlock(
      const juce::AudioSourceChannelInfo& bufferToFill) override;

  /**
   * @brief Try to reserve MIDI message after triggering.
   * @param[in] message MIDI message
   * @return \c true if given message was used. If it was discareded, returns
   * \c false.
   */
  bool tryMidiMessageReservation(const juce::MidiMessage& message);

  /**
   * @brief Change audio source state by executing reserved MIDI messages and
   * some changes.
   */
  void triggerReservedMidiMessages();

 private:
  /// Emutator.
  std::unique_ptr<ymfm::ym2608> ym2608_;

  /// Emulator interface.
  ymfm::ymfm_interface interface_;

  /// Temporary buffer to store samples generated by the emulator.
  std::vector<ymfm::ym2608::output_data> outputDataBuffer_;

  /// Manager of note-on and -off.
  Keyboard keyboard_;

  /**
   * @brief Value pair of address and data of register.
   */
  struct Register {
    /// State of pin A1 which controls data bus in a chip.
    bool pinA1{};

    /// Address where data should be written in register.
    std::uint8_t address{};

    /// Data to write.
    std::uint8_t data{};

    /**
     * @brief Constructor.
     * @param[in] pinA1 State of pin A1.
     * @param[in] address Address where data should be written in register.
     * @param[in] data Data to write.
     */
    Register(bool pinA1, std::uint8_t address, std::uint8_t data)
        : pinA1(pinA1), address(address), data(data){};

    /**
     * @brief Constructor supporting 16-bit expression for address.
     * @param[in] address Bit 0-7 is address. Bit 8 is a flag which represents a
     * state of pin A1.
     * @param[in] data Data to write.
     */
    Register(std::uint16_t address, std::uint8_t data)
        : pinA1(address & 0x0100),
          address(static_cast<std::uint8_t>(address & 0x00ff)),
          data(data) {}
  };

  /// Queue of register changes.
  std::vector<Register> reservedChanges_;

  /**
   * @brief Reserve register changes related on note-on event.
   * @param[in] assignment Details of note-on event.
   * @return \c true if the reservation is success, otherwise \c false.
   */
  bool reserveNoteOn(const NoteAssignment& assignment);

  /**
   * @brief Reserve register changes related on note-off event.
   * @param[in] assignment Details of note-off event.
   * @return \c true if the reservation is success, otherwise \c false.
   */
  bool reserveNoteOff(const NoteAssignment& assignment);

  void setTone();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FmAudioSource)
};
}  // namespace audio
