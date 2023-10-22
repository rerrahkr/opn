// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "fm_operator_parameters_tab_content.h"

#include "../audio/parameter/parameter.h"
#include "nestable_grid.h"

namespace ui {
FmOperatorParametersTabContent::FmOperatorParametersTabContent(
    std::size_t slotId, juce::AudioProcessorValueTreeState& parameters) {
  namespace ap = audio::parameter;

  // Helper function of addAndMakeVisible().
  const auto addAndMakeVisibleButton = [this](auto& pair) {
    addAndMakeVisible(pair->label.get());
    addAndMakeVisible(pair->toggleButton.get());
  };

  const auto addAndMakeVisibleSlider = [this](auto& pair) {
    addAndMakeVisible(pair->label.get());
    addAndMakeVisible(pair->slider.get());
  };

  enabledPair_ = std::make_unique<LabeledToggleButtonWithAttachment>(
      parameters,
      ap::idAsString(slotId, ap::FmOperatorParameter::OperatorEnabled),
      "Enabled");
  addAndMakeVisibleButton(enabledPair_);

  arPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Ar),
      "Attack Rate", juce::Slider::LinearHorizontal,
      juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(arPair_);

  tlPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Tl),
      "Total Level", juce::Slider::LinearHorizontal,
      juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(tlPair_);

  drPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Dr),
      "Decay Rate", juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(drPair_);

  slPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Sl),
      "Sustain Level", juce::Slider::LinearHorizontal,
      juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(slPair_);

  srPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Sr),
      "Sustain Rate", juce::Slider::LinearHorizontal,
      juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(srPair_);

  rrPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Rr),
      "Release Rate", juce::Slider::LinearHorizontal,
      juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(rrPair_);

  ksPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Ks),
      "Key Scale", juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(ksPair_);

  mlPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Ml),
      "Multiple", juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(mlPair_);

  dtPair_ = std::make_unique<LabeledSliderWithAttachment>(
      parameters, ap::idAsString(slotId, ap::FmOperatorParameter::Dt), "Detune",
      juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
  addAndMakeVisibleSlider(dtPair_);
}

FmOperatorParametersTabContent::~FmOperatorParametersTabContent() = default;

void FmOperatorParametersTabContent::resized() {
  NestableGrid grid;
  grid.setTemplateColumns({1_fr, 1_fr});
  grid.setTemplateRows(
      {1_fr, 1_fr, 1_fr, 1_fr, 1_fr, 1_fr, 1_fr, 1_fr, 1_fr, 1_fr});
  grid.setItems({
      enabledPair_->label.get(), enabledPair_->toggleButton.get(),
      arPair_->label.get(),      arPair_->slider.get(),
      tlPair_->label.get(),      tlPair_->slider.get(),
      drPair_->label.get(),      drPair_->slider.get(),
      slPair_->label.get(),      slPair_->slider.get(),
      srPair_->label.get(),      srPair_->slider.get(),
      rrPair_->label.get(),      rrPair_->slider.get(),
      ksPair_->label.get(),      ksPair_->slider.get(),
      mlPair_->label.get(),      mlPair_->slider.get(),
      dtPair_->label.get(),      dtPair_->slider.get(),
  });
  grid.performLayout(getLocalBounds());
}
}  // namespace ui
