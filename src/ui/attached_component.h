// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <functional>

#include "animated_toggle_button.h"

namespace ui {
/**
 * @brief The pair of label and slider with attachment for plugin parameter.
 */
struct LabeledSliderWithAttachment {
  std::unique_ptr<juce::Label> label;
  std::unique_ptr<juce::Slider> slider;
  juce::AudioProcessorValueTreeState::SliderAttachment attachment;

  /**
   * @brief Constructor.
   * @param[in] parameters Value tre of plugin parameters.
   * @param[in] parameterId ID of attached parameter.
   * @param[in] labelText Text displayed on label.
   * @param[in] args Constructor arguments for slider.
   */
  template <class... Args>
  LabeledSliderWithAttachment(juce::AudioProcessorValueTreeState& parameters,
                              const juce::String& parameterId,
                              const juce::String& labelText, Args&&... args)
      : label(std::make_unique<juce::Label>("", labelText)),
        slider(std::make_unique<juce::Slider>(std::forward<Args>(args)...)),
        attachment(parameters, parameterId, *slider){};
};

/**
 * @brief The pair of label and toggle button with attachment for plugin
 * parameter.
 */
struct LabeledToggleButtonWithAttachment {
  std::unique_ptr<juce::Label> label;
  std::unique_ptr<AnimatedToggleButton> toggleButton;
  juce::AudioProcessorValueTreeState::ButtonAttachment attachment;

  /**
   * @brief Constructor.
   * @param[in] parameters Value tre of plugin parameters.
   * @param[in] parameterId ID of attached parameter.
   * @param[in] labelText Text displayed on label.
   */
  LabeledToggleButtonWithAttachment(
      juce::AudioProcessorValueTreeState& parameters,
      const juce::String& parameterId, const juce::String& labelText)
      : label(std::make_unique<juce::Label>("", labelText)),
        toggleButton(std::make_unique<AnimatedToggleButton>()),
        attachment(parameters, parameterId, *toggleButton){};
};
}  // namespace ui
