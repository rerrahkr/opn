// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.cpp

#include "plugin_editor.h"

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
  {
    pitchBendSensitivitySlider_ = std::make_unique<juce::Slider>(
        juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft);
    pitchBendSensitivitySliderAttachment_ = std::make_unique<SliderAttachment>(
        parameters,
        plugin_parameter::idAsString(
            plugin_parameter::Type::PitchBendSensitivity),
        *pitchBendSensitivitySlider_);
    addAndMakeVisible(pitchBendSensitivitySlider_.get());
  }

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
  pitchBendSensitivitySlider_->setBounds(0, 0, 120, 20);
}

void PluginEditor::actionListenerCallback(const String& /*message*/) {}
