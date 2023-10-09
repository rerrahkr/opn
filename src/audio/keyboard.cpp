// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "keyboard.h"

#include <algorithm>
#include <numeric>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace audio {
Keyboard::Keyboard(std::size_t polyphony)
    : polyphony_(polyphony), assignableIdQueue_(polyphony) {
  if (!polyphony) {
    throw std::invalid_argument("Polyphony must be greater than zero.");
  }

  std::iota(std::begin(assignableIdQueue_), std::end(assignableIdQueue_), 0u);
}

std::deque<NoteAssignment> Keyboard::setPolyphony(std::size_t newPolyphony) {
  if (!newPolyphony) {
    throw std::invalid_argument("Polyphony must be greater than zero.");
  }

  const std::size_t oldPolyphony = std::exchange(polyphony_, newPolyphony);

  if (newPolyphony < oldPolyphony) {
    // Shrink polyphony.
    std::size_t decreasedSize = newPolyphony - oldPolyphony;
    if (std::size(noteOnQueue_) + std::size(assignableIdQueue_) <
        decreasedSize) {
      throw std::range_error("Polyphony state is broken.");
    }

    // Reduce polyphony from assignable ID.
    const std::size_t nDeletableAssignableId =
        std::min(decreasedSize, std::size(assignableIdQueue_));
    assignableIdQueue_.erase(
        std::cbegin(assignableIdQueue_),
        std::cbegin(assignableIdQueue_) + nDeletableAssignableId);
    decreasedSize -= nDeletableAssignableId;
    if (!decreasedSize) {
      return {};
    }

    // Reduce polyphony from older note-ons.
    std::deque<NoteAssignment> noteOffQueue;
    std::transform(
        std::cbegin(noteOnQueue_), std::cbegin(noteOnQueue_) + decreasedSize,
        std::back_inserter(noteOffQueue), [](const NoteAssignment& assignment) {
          return NoteAssignment{
              .assignId{assignment.assignId},
              .note{Note::noteOff(assignment.note.channel,
                                  assignment.note.noteNumber)}};
        });
    noteOnQueue_.erase(std::cbegin(noteOnQueue_),
                       std::cbegin(noteOnQueue_) + decreasedSize);
    return noteOffQueue;
  } else if (oldPolyphony < newPolyphony) {
    // Aggregate current used IDs.
    std::set<std::size_t> usedIds(std::cbegin(assignableIdQueue_),
                                  std::cend(assignableIdQueue_));
    std::transform(
        std::cbegin(noteOnQueue_), std::cend(noteOnQueue_),
        std::inserter(usedIds, std::end(usedIds)),
        [](const NoteAssignment& assignment) { return assignment.assignId; });
    if (std::size(usedIds) != oldPolyphony) {
      throw std::range_error("Polyphony state is broken.");
    }

    // Get difference between used IDs and polyphony number sequence.
    std::vector<std::size_t> polyphonies(newPolyphony);
    std::iota(std::begin(polyphonies), std::end(polyphonies), 0u);

    std::vector<std::size_t> difference;
    std::set_difference(std::cbegin(polyphonies), std::cend(polyphonies),
                        std::cbegin(usedIds), std::cend(usedIds),
                        std::back_inserter(difference));

    // Add new IDs from the difference.
    std::size_t increasedSize = newPolyphony - oldPolyphony;
    if (std::size(difference) < increasedSize) {
      throw std::range_error("Polyphony state is broken.");
    }
    assignableIdQueue_.insert(std::cbegin(assignableIdQueue_),
                              std::begin(difference),
                              std::begin(difference) + increasedSize);

    return {};
  } else {
    return {};
  }
}

std::set<std::size_t> Keyboard::usedAssignIds() const {
  std::set<std::size_t> ids;
  std::transform(
      std::cbegin(noteOnQueue_), std::cend(noteOnQueue_),
      std::inserter(ids, std::end(ids)),
      [](const NoteAssignment& assignment) { return assignment.assignId; });
  ids.insert(std::cbegin(assignableIdQueue_), std::cend(assignableIdQueue_));
  return ids;
}

std::deque<NoteAssignment> Keyboard::tryNoteOn(const Note& note) {
  if (note.isNoteOn() == false) {
    return {};
  }

  std::deque<NoteAssignment> changes;

  // Note off if there is any note which has the same note number.
  if (const auto&& noteOffAssignment = tryNoteOff(note);
      noteOffAssignment.has_value()) {
    changes.push_back(std::move(noteOffAssignment.value()));
  }

  if (assignableIdQueue_.empty()) {
    // Force oldest note to do note-off.
    const auto& oldest = noteOnQueue_.front();
    changes.push_back(NoteAssignment{
        .assignId{oldest.assignId},
        .note{Note::noteOff(oldest.note.channel, oldest.note.noteNumber)}});
    assignableIdQueue_.push_back(oldest.assignId);
    noteOnQueue_.pop_front();
  }

  // Assign new note-on.
  const NoteAssignment newAssignment{.assignId{assignableIdQueue_.front()},
                                     .note{note}};
  assignableIdQueue_.pop_front();
  noteOnQueue_.push_back(newAssignment);
  changes.push_back(newAssignment);

  return changes;
}

std::optional<NoteAssignment> Keyboard::tryNoteOff(const Note& note) {
  if (note.isNoteOn()) {
    return {};
  }

  const auto sameNoteItr =
      std::find_if(std::cbegin(noteOnQueue_), std::cend(noteOnQueue_),
                   [&note](const NoteAssignment& assignment) {
                     return (assignment.note.channel == note.channel) &&
                            (assignment.note.noteNumber == note.noteNumber);
                   });
  if (sameNoteItr == std::cend(noteOnQueue_)) {
    return {};
  }

  assignableIdQueue_.push_back(sameNoteItr->assignId);
  const NoteAssignment noteOffAssignment{
      .assignId{sameNoteItr->assignId},
      .note{Note::noteOff(sameNoteItr->note.channel,
                          sameNoteItr->note.noteNumber)}};
  noteOnQueue_.erase(sameNoteItr);

  return noteOffAssignment;
}

std::deque<NoteAssignment> Keyboard::forceAllNoteOff() {
  std::deque<NoteAssignment> noteOffQueue;
  std::transform(
      std::cbegin(noteOnQueue_), std::cend(noteOnQueue_),
      std::back_inserter(noteOffQueue), [](const NoteAssignment& assignment) {
        return NoteAssignment{.assignId{assignment.assignId},
                              .note{Note::noteOff(assignment.note.channel,
                                                  assignment.note.noteNumber)}};
      });
  noteOnQueue_.clear();
  return noteOffQueue;
}
}  // namespace audio
