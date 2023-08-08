// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

/**
 * @brief Interface of model.
 */
struct EditorState {};

/**
 * @brief Model class.
 */
class Model : public juce::ActionBroadcaster, public EditorState {
 public:
  Model() noexcept = default;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Model);
};
