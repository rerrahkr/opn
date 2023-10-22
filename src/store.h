// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <concepts>
#include <functional>
#include <utility>
#include <vector>

/**
 * @brief Store class.
 * @tparam State State class.
 * @tparam Action Action class which needs to have two members: @c type and \c
 * payload.
 */
template <class State, class Action>
  requires requires(Action a) {
    a.type;
    a.payload;
  }
class PluginStore {
 public:
  /**
   * @brief Construct a new Plugin Store object
   * @param[in] reducer
   */
  PluginStore(std::function<State(const State&, const Action&)> reducer)
      : reducer_(reducer) {}

  /**
   * @brief Subscribe state changes.
   * @param[in] callback Function called when the state is changed. Note that
   * there is no guarantee that this callback will be invoked in the message
   * thread.
   */
  void subscribe(std::function<void(const State&)> callback) {
    subscribers_.emplace_back(std::move(callback));
  }

  /**
   * @brief Dispatch an acton to update the state.
   * @param[in] action Dispatched action.
   */
  void dispatch(const Action& action) {
    if (!reducer_) {
      return;
    }

    state_ = reducer_(state_, action);

    for (auto& subscriber : subscribers_) {
      subscriber(state_);
    }
  }

 private:
  State state_;
  std::function<State(const State&, const Action&)> reducer_;
  std::vector<std::function<void(const State&)>> subscribers_;
};
