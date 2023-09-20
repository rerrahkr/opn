// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "parameter_change_queue.h"

#include <stdexcept>

namespace audio {
namespace parameter {
void ParameterChangeQueue::enqueue(
    const audio::parameter::ParameterVariant& parameter) {
  const std::size_t typeIndex = parameter.index();

  if (const auto itr = map_.find(typeIndex); itr != std::end(map_)) {
    queue_.erase(itr->second);
  }

  queue_.emplace_front(parameter);
  map_.insert_or_assign(typeIndex, std::begin(queue_));
}

audio::parameter::ParameterVariant ParameterChangeQueue::dequeue() {
  if (queue_.empty()) {
    throw std::range_error("Called dequeue, but the queue is empty.");
  }

  auto back = std::move(queue_.back());

  map_.erase(back.index());
  queue_.pop_back();

  return back;
}

void ParameterChangeQueue::clear() {
  map_.clear();
  queue_.clear();
}
}  // namespace parameter
}  // namespace audio
