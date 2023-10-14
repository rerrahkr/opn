// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <functional>

namespace ui {
/**
 * @brief Pair of slider and its attachment.
 */
struct AttachedSlider : public AudioProcessorValueTreeState::Listener {
  juce::Slider slider;
  juce::AudioProcessorValueTreeState::SliderAttachment attachment;
  std::function<void(float)> onValueChanged;

  /**
   * @brief Constructor.
   *
   * @param[in] parameters Plugin parameters.
   * @param[in] parameterId Parameter identifier.
   * @param[in] onValueChanged Function called when the parameter value is
   * changed.
   */
  AttachedSlider(juce::AudioProcessorValueTreeState& parameters,
                 const juce::String& parameterId,
                 const std::function<void(float)> onValueChanged)
      : attachment(parameters, parameterId, slider),
        onValueChanged(onValueChanged) {
    parameters.addParameterListener(parameterId, this);
  }

  /**
   * @brief Constructor with slider styles.
   *
   * @param[in] style Slider style.
   * @param[in] textBoxPosition Position of text box in slider.
   * @param[in] parameters Plugin parameters.
   * @param[in] parameterId Parameter identifier.
   * @param[in] onValueChanged Function called when the parameter value is
   * changed.
   */
  AttachedSlider(juce::Slider::SliderStyle style,
                 juce::Slider::TextEntryBoxPosition textBoxPosition,
                 juce::AudioProcessorValueTreeState& parameters,
                 const juce::String& parameterId,
                 const std::function<void(float)> onValueChanged)
      : slider(style, textBoxPosition),
        attachment(parameters, parameterId, slider),
        onValueChanged(onValueChanged) {
    parameters.addParameterListener(parameterId, this);
  }

  void parameterChanged(const String& /*parameterID*/,
                        float newValue) override {
    // TODO: Prefer using juce::ChangerBroadCaster to get juce::MessageManager
    // for frequent updates.
    auto* messageManager = juce::MessageManager::getInstanceWithoutCreating();
    if (messageManager && onValueChanged) {
      if (messageManager->isThisTheMessageThread()) {
        onValueChanged(newValue);
      } else {
        juce::MessageManager::callAsync(
            [newValue, f = onValueChanged] { f(newValue); });
      }
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AttachedSlider);
};

/**
 * @brief Pair of toggle button and its attachment.
 */
struct AttachedToggleButton
    : public juce::AudioProcessorValueTreeState::Listener {
  juce::ToggleButton button;
  juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
  std::function<void(float)> onValueChanged;

  /**
   * @brief Constructor.
   *
   * @param[in] parameters Plugin parameters.
   * @param[in] parameterId Parameter identifier.
   * @param[in] onValueChanged Function called when the parameter value is
   * changed.
   */
  AttachedToggleButton(juce::AudioProcessorValueTreeState& parameters,
                       const juce::String& parameterId,
                       const std::function<void(float)> onValueChanged)
      : attachment(parameters, parameterId, button),
        onValueChanged(onValueChanged) {
    parameters.addParameterListener(parameterId, this);
  }

  void parameterChanged(const String& /*parameterID*/,
                        float newValue) override {
    // TODO: Prefer using juce::ChangerBroadCaster to get juce::MessageManager
    // for frequent updates.
    auto* messageManager = juce::MessageManager::getInstanceWithoutCreating();
    if (messageManager && onValueChanged) {
      if (messageManager->isThisTheMessageThread()) {
        onValueChanged(newValue);
      } else {
        juce::MessageManager::callAsync(
            [newValue, f = onValueChanged] { f(newValue); });
      }
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AttachedToggleButton);
};
}  // namespace ui
