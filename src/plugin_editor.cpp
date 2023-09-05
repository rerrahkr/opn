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

  // Pitch bend sensitivity.
  pitchBendSensitivityLabel_ = makeLabel("Pitch bend sensitivity");
  addAndMakeVisible(pitchBendSensitivityLabel_.get());
  pitchBendSensitivitySlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      audio::parameter::idAsString(
          audio::parameter::PluginParameter::PitchBendSensitivity),
      [&processor] {
        processor.reserveParameterChange(
            audio::parameter::PluginParameter::PitchBendSensitivity);
      });
  addAndMakeVisible(pitchBendSensitivitySlider_->slider);

  // Feedback.
  fbLabel_ = makeLabel("Feedback");
  addAndMakeVisible(fbLabel_.get());
  fbSlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight, parameters,
      audio::parameter::idAsString(audio::parameter::FmToneParameter::Fb),
      [&processor] {
        processor.reserveParameterChange(audio::parameter::FmToneParameter::Fb);
      });
  addAndMakeVisible(fbSlider_->slider);

  // Algorithm.
  alSlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      audio::parameter::idAsString(audio::parameter::FmToneParameter::Al),
      [&processor] {
        processor.reserveParameterChange(audio::parameter::FmToneParameter::Al);
      });
  addAndMakeVisible(alSlider_->slider);

  // Operator 1.

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
  constexpr int kRowHeight{20}, kSliderWidth{120};
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
}

void PluginEditor::actionListenerCallback(const String& /*message*/) {}
