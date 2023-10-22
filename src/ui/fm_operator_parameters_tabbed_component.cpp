// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "fm_operator_parameters_tabbed_component.h"

#include <utility>

namespace ui {
FmOperatorParametersTabbedComponent::FmOperatorParametersTabbedComponent(
    juce::TabbedButtonBar::Orientation orientation,
    std::function<void(int)> callback)
    : juce::TabbedComponent(orientation), callback_(std::move(callback)) {}

FmOperatorParametersTabbedComponent::~FmOperatorParametersTabbedComponent() =
    default;

void FmOperatorParametersTabbedComponent::currentTabChanged(
    int newCurrentTabIndex, const String& /*newCurrentTabName*/) {
  if (callback_) {
    callback_(newCurrentTabIndex);
  }
}
}  // namespace ui
