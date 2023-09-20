// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <list>
#include <unordered_map>

#include "parameter.h"

namespace audio {
namespace parameter {
/**
 * @brief FIFO queue of parameters whoose element type is unique.
 */
class ParameterChangeQueue {
 public:
  /**
   * @brief Enqueue parameter.
   *
   * @param[in] parameter Parameter.
   */
  void enqueue(const ParameterVariant& parameter);

  /**
   * @brief Dequeue parameter.
   *
   * @return Parameter.
   * @exception @c std::range_error if the queue is empty.
   */
  ParameterVariant dequeue();

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
  std::list<ParameterVariant> queue_;

  /**
   * @brief Unique element map of queue.
   *        Key is index of type in parameter value variant.
   *        Value is iterator of parameter value in @c queue_.
   */
  std::unordered_map<std::size_t, decltype(queue_)::iterator> map_;
};
}  // namespace parameter
}  // namespace audio
