// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <cmath>
#include <cstdint>

namespace audio {
namespace pitch_util {
/// MIDI note number definitions.
constexpr inline int kC4NoteNumber{60}, kA4NoteNumber{kC4NoteNumber + 9};

/// Note frequency definition.
constexpr inline double kA4Hz{440.};

/// Octave definition.
constexpr inline int kSemitoneCountInOctave{12};

/// Cent of semitone.
constexpr inline int kSemitoneCent{100};

/// Range of pitch bend.
constexpr inline int kMinPitchBend{-8192}, kMaxPitchBend{8191};

/**
 * @brief Calculate MIDI note cent.
 * @param[in] noteNumber MIDI note number.
 * @param[in] pitchBend Pitch bend value.
 * @param[in] pitchBendSensitivity Pitch bend sensitivity.
 * @return Cent from MIDI note number 0.
 */
inline int calculateCent(int noteNumber, int pitchBend,
                         int pitchBendSensitivity) {
  return noteNumber * kSemitoneCent +
         kSemitoneCent * pitchBendSensitivity * pitchBend /
             ((pitchBend < 0) ? -kMinPitchBend : kMaxPitchBend);
}

/**
 * @brief Calculate frequency from cent.
 * @param[in] cent Cent from MIDI note number 0.
 * @return Frequency (Hz.)
 */
inline double calculateHzFromCent(int cent) {
  return kA4Hz * std::pow(2.0, static_cast<double>(cent - kA4NoteNumber *
                                                              kSemitoneCent) /
                                   (kSemitoneCountInOctave * kSemitoneCent));
}
}  // namespace pitch_util
}  // namespace audio
