// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.cpp

#include "plugin_editor.h"

#include "audio/parameter.h"
#include "controller.h"
#include "model.h"
#include "plugin_processor.h"
#include "ui/nestable_grid.h"
#include "ui/view_message.h"

//==============================================================================
PluginEditor::PluginEditor(PluginProcessor& processor, Controller& controller,
                           const EditorState& model,
                           juce::AudioProcessorValueTreeState& parameters)
    : AudioProcessorEditor(&processor), controller_(controller), model_(model) {
  const auto makeLabel = [](const juce::String& text) {
    return std::make_unique<juce::Label>("", text);
  };

  namespace ap = audio::parameter;

  // Pitch bend sensitivity.
  pitchBendSensitivityLabel_ = makeLabel("Pitch bend sensitivity");
  addAndMakeVisible(pitchBendSensitivityLabel_.get());
  pitchBendSensitivitySlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      ap::idAsString(ap::PluginParameter::PitchBendSensitivity), [&processor] {
        processor.reserveParameterChange(
            ap::PluginParameter::PitchBendSensitivity);
      });
  addAndMakeVisible(pitchBendSensitivitySlider_->slider);

  // Feedback.
  fbLabel_ = makeLabel("Feedback");
  addAndMakeVisible(fbLabel_.get());
  fbSlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight, parameters,
      ap::idAsString(ap::FmToneParameter::Fb), [&processor] {
        processor.reserveParameterChange(ap::FmToneParameter::Fb);
      });
  addAndMakeVisible(fbSlider_->slider);

  // Algorithm.
  alSlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      ap::idAsString(ap::FmToneParameter::Al), [&processor] {
        processor.reserveParameterChange(ap::FmToneParameter::Al);
      });
  addAndMakeVisible(alSlider_->slider);

  // Operator parameters.
  for (std::size_t i = 0; i < audio::FmParameters::kSlotCount; ++i) {
    auto& sliderMap = operatorSliders_[i];

    using enum ap::FmOperatorParameter;
    constexpr ap::FmOperatorParameter parameterTypes[]{
        Ar, Dr, Sr, Rr, Sl, Tl, Ks, Ml, Dt,
    };

    for (const auto parameterType : parameterTypes) {
      [[maybe_unused]] auto [iter, _] = sliderMap.emplace(
          std::piecewise_construct, std::forward_as_tuple(parameterType),
          std::forward_as_tuple(
              juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight,
              parameters, ap::idAsString(i, parameterType),
              [&processor,
               pair = ap::FmOperatorParameterWithSlot(i, parameterType)] {
                processor.reserveParameterChange(pair);
              }));
      addAndMakeVisible(iter->second.slider);
    }

    auto enabledButton = std::make_unique<ui::AttachedToggleButton>(
        parameters, ap::idAsString(i, OperatorEnabled),
        [&processor,
         pair = ap::FmOperatorParameterWithSlot(i, OperatorEnabled)] {
          processor.reserveParameterChange(pair);
        });
    addAndMakeVisible(enabledButton->button);
    operatorEnabledButtons_[i] = std::move(enabledButton);
  }

  setSize(400, 300);
  setResizable(true, false);
  resized();
}

PluginEditor::~PluginEditor() {}

//==============================================================================
void PluginEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized() {
  constexpr int kRowHeight{20}, kSliderWidth{120}, kToggleButtonWidth{20};
  int y{};

  pitchBendSensitivityLabel_->setBounds(0, y, 200, kRowHeight);
  pitchBendSensitivitySlider_->slider.setBounds(200, y, kSliderWidth,
                                                kRowHeight);
  y += kRowHeight;

  fbLabel_->setBounds(0, y, 100, kRowHeight);
  fbSlider_->slider.setBounds(100, y, kSliderWidth, kRowHeight);
  y += kRowHeight;

  alSlider_->slider.setBounds(0, y, kSliderWidth, kRowHeight);
  y += kRowHeight;

  for (auto& sliderMap : operatorSliders_) {
    for ([[maybe_unused]] auto& [_, slider] : sliderMap) {
      slider.slider.setBounds(0, y, kSliderWidth + 100, kRowHeight);
      y += kRowHeight;
    }
  }

  for (auto& button : operatorEnabledButtons_) {
    button->button.setBounds(0, y, kToggleButtonWidth, kRowHeight);
    y += kRowHeight;
  }
}

void PluginEditor::actionListenerCallback(const String& /*message*/) {}
