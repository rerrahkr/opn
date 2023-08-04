// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <set>

#include "note.h"

namespace audio {
/**
 * @brief Information of note assignment on polyphony management.
 */
struct NoteAssignment {
  std::size_t assignId;  ///< Identifier of assignment place.
  Note note;             ///< Note.
};

/**
 * @brief A class to manage note-on and polyphony following FIFO strategy.
 */
class Keyboard {
 public:
  /**
   * @brief Constructor.
   * @param[in] polyphony A number of polyphony. It must be greater than zero.
   * @exception \c std::invalid_argument.
   */
  Keyboard(std::size_t polyphony);

  /**
   * @brief Set the number of polyphony.
   * @param[in] newPolyphony The maximum number of note that is able to note on
   * at the same time. It must be greater than zero.
   * @return A list of note assignment which should be note-on by polyphony
   * change.
   * @exception \c std::invalid_argument if \c newPolyphony is set to
   * zero.
   * @exception \c std::range_error if internal polyphony state is broken.
   */
  std::deque<NoteAssignment> setPolyphony(std::size_t newPolyphony);

  /**
   * @brief Get the number of polyphony.
   * @return The number of polyphony.
   */
  std::size_t polyphony() const noexcept { return polyphony_; }

  /**
   * @brief Get a list of identifier which used and kept for note-on.
   * @return A list of all identifier.
   */
  std::set<std::size_t> usedAssignIds() const;

  /**
   * @brief Try note on.
   * @param[in] note A note to try note-on.
   * @return A list of assignment infomation which should be note-on or off.
   */
  std::deque<NoteAssignment> tryNoteOn(const Note& note);

  /**
   * @brief Try note-off.
   * @param[in] note A note to try note-off.
   * @return An assignment information which should be note off.
   */
  std::optional<NoteAssignment> tryNoteOff(const Note& note);

  /**
   * @brief Do note-off for all notes.
   * @return A list of note assignment which should do note-off.
   */
  std::deque<NoteAssignment> forceAllNoteOff();

 private:
  /// A queue of note which is note-on.
  std::deque<NoteAssignment> noteOnQueue_;

  /// A queue of identifier which is assignable to a note.
  std::deque<std::size_t> assignableIdQueue_;

  /// The maximum number of note that is able to note on at the same time.
  std::size_t polyphony_;
};
}  // namespace audio
