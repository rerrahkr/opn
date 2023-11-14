// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <memory>

namespace ui {
/**
 * @brief Graph displayed FM algorithm.
 */
class AlgorithmGraph : public juce::Component {
 public:
  /**
   * @brief Constructor.
   * @param[in] parameters Parameters of plugin.
   */
  AlgorithmGraph(juce::AudioProcessorValueTreeState& parameters);

  /**
   * @brief Update displayed image.
   */
  void update();

  void paint(juce::Graphics& graphics) override;
  void resized() override;

 private:
  /// Parameters of plugin.
  juce::AudioProcessorValueTreeState& parameters_;

  /// Displayed SVG image.
  std::unique_ptr<juce::Drawable> svg_;
};
}  // namespace ui
