// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <list>
#include <stdexcept>
#include <unordered_map>

#include "audio/parameter.h"

namespace audio {
class FmAudioSource;
}

/**
 * @brief FIFO queue of parameters whoose element is unique.
 */
class ParameterChangeQueue {
 public:
  /**
   * @brief Enqueue parameter.
   *
   * @param[in] parameter Parameter.
   */
  void enqueue(const audio::Parameter& parameter);

  /**
   * @brief Dequeue parameter.
   *
   * @return Parameter.
   * @exception @c std::range_error if the queue is empty.
   */
  audio::Parameter dequeue();

  /**
   * @brief Clear the queue.
   */
  void clear();

  /**
   * @brief Whether the queue has no element.
   *
   * @return @c true if the queue has no element, otherwise @c false.
   */
  bool empty() const noexcept { return queue_.empty(); }

 private:
  /// Front-In, Back-Out queue.
  std::list<audio::Parameter> queue_;

  /// Unique element map of queue.
  std::unordered_map<audio::Parameter, decltype(queue_.begin())> map_;
};
