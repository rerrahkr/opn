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
  // Pitch bend sensitivity.
  pitchBendSensitivitySlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      audio::parameter::idAsString(
          audio::parameter::PluginParameter::PitchBendSensitivity),
      [&processor] {
        processor.reserveParameterChange(
            audio::parameter::PluginParameter::PitchBendSensitivity);
      });
  addAndMakeVisible(pitchBendSensitivitySlider_->slider);

  setSize(400, 300);
  resized();
}

PluginEditor::~PluginEditor() {}

//==============================================================================
void PluginEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized() {
  pitchBendSensitivitySlider_->slider.setBounds(0, 0, 120, 20);
}

void PluginEditor::actionListenerCallback(const String& /*message*/) {}
