// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "animated_toggle_button.h"

namespace ui {
AnimatedToggleButton::~AnimatedToggleButton() = default;

void AnimatedToggleButton::buttonStateChanged() {
  if (isTimerRunning()) {
    return;
  }

  startTimerHz(60);
}

void AnimatedToggleButton::paintButton(juce::Graphics& graphics,
                                       bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown) {
  const auto bounds = getLocalBounds().toFloat();
  const auto diameter = bounds.getHeight();
  const auto radius = diameter * .5f;
  auto h = float(getHeight());

  juce::Path path;  // leaving as a path so an optional outline can be added
  path.addRoundedRectangle(bounds, radius);
  graphics.setColour(
      juce::Colours::darkgrey.interpolatedWith(juce::Colours::green, portion_));
  graphics.fillPath(path);

  const auto circleBounds =
      juce::Rectangle{(bounds.getWidth() - diameter) * portion_, 0.f, diameter,
                      diameter}
          .reduced(radius * .1f);
  graphics.setColour(juce::Colours::lightgrey);
  graphics.fillEllipse(circleBounds);
}

void AnimatedToggleButton::timerCallback() {
  portion_ =
      juce::jlimit(0.f, 1.f, portion_ + .1f * (getToggleState() ? 1.f : -1.f));

  if (portion_ == 0.f || portion_ == 1.f) {
    stopTimer();
  }

  repaint();
}
}  // namespace ui
