// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <functional>

namespace ui {
/**
 * @brief Tabbed component for FM operator parameters.
 */
class FmOperatorParametersTabbedComponent : public juce::TabbedComponent {
 public:
  /**
   * @brief Constructor.
   * @param[in] orientation Tab orientation.
   * @param[in] callback Function called when current tab is changed.
   */
  FmOperatorParametersTabbedComponent(
      juce::TabbedButtonBar::Orientation orientation,
      std::function<void(int)> callback);

  ~FmOperatorParametersTabbedComponent();

  void currentTabChanged(int newCurrentTabIndex,
                         const String& /*newCurrentTabName*/) override;

 private:
  std::function<void(int)> callback_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      FmOperatorParametersTabbedComponent)
};
}  // namespace ui
