// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <cstddef>

/// State of plugin except for plugin parameters.
struct PluginState {
  std::size_t envelopeGraphFrontSlot{};
};
