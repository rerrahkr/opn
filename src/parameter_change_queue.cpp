// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "parameter_change_queue.h"

void ParameterChangeQueue::enqueue(const audio::Parameter& parameter) {
  if (const auto itr = map_.find(parameter); itr != std::end(map_)) {
    queue_.erase(itr->second);
  }

  queue_.emplace_front(parameter);
  map_.insert_or_assign(parameter, queue_.begin());
}

audio::Parameter ParameterChangeQueue::dequeue() {
  if (queue_.empty()) {
    throw std::range_error("Called dequeue, but the queue is empty.");
  }

  auto back = std::move(queue_.back());

  map_.erase(back);
  queue_.pop_back();

  return back;
}

void ParameterChangeQueue::clear() {
  map_.clear();
  queue_.clear();
}
