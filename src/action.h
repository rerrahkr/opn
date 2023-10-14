// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <any>

/**
 * @brief Action data.
 */
struct PluginAction {
  /// Enumeration of action type.
  enum class Type {
    InvalidAction,

    EnvelopeGraphFrontRadioButtonChanged,
  };

  /// Type of action.
  Type type{Type::InvalidAction};

  /// Payload stored data which is related to its action.
  std::any payload{};
};
