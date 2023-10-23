// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

namespace ui {
class AnimatedToggleButton : public juce::ToggleButton, private juce::Timer {
 public:
  AnimatedToggleButton() = default;
  ~AnimatedToggleButton();

 protected:
  void buttonStateChanged() override;

  void paintButton(juce::Graphics& graphics, bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override;

 private:
  /// Current position of circle.
  float portion_{};

  void timerCallback() override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedToggleButton)
};
}  // namespace ui
