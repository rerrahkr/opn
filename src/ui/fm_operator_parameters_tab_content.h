// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <memory>

#include "attached_component.h"

namespace ui {
/**
 * @brief Content of tabbed component for FM operator parameters.
 */
class FmOperatorParametersTabContent : public juce::Component {
 public:
  /**
   * @brief Constructor.
   * @param[in] slotId Operator number.
   * @param[in] parameters Plugin parameters.
   */
  FmOperatorParametersTabContent(
      std::size_t slotId, juce::AudioProcessorValueTreeState& parameters);

  ~FmOperatorParametersTabContent();

  void resized() override;

 private:
  std::unique_ptr<LabeledToggleButtonWithAttachment> enabledPair_;
  std::unique_ptr<LabeledSliderWithAttachment> arPair_, tlPair_, drPair_,
      slPair_, srPair_, rrPair_, ksPair_, mlPair_, dtPair_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FmOperatorParametersTabContent)
};
}  // namespace ui
