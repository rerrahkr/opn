// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <variant>

#include "action.h"
#include "state.h"

/**
 * @brief Reducer functor.
 */
struct PluginReducer {
  /**
   * @brief Do reducer operation.
   * @param[in] oldState State before reducer execution.
   * @param[in] action Action.
   * @return New state changed from \c oldState by handling @c action.
   */
  PluginState operator()(const PluginState& oldState,
                         const PluginAction& action);
};
