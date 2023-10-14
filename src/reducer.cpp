// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "reducer.h"

PluginState PluginReducer::operator()(
    [[maybe_unused]] const PluginState& oldState, const PluginAction& action) {
  PluginState newState{};

  switch (action.type) {
    using enum PluginAction::Type;

    default:
    case InvalidAction:
      break;

    case EnvelopeGraphFrontRadioButtonChanged:
      if (action.payload.type() ==
          typeid(decltype(newState.envelopeGraphFrontSlot))) {
        newState.envelopeGraphFrontSlot =
            std::any_cast<decltype(newState.envelopeGraphFrontSlot)>(
                action.payload);
      }
      break;
  }

  return newState;
}
