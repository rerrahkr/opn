// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.h

#pragma once

#include <JuceHeader.h>

#include <memory>

class Controller;
struct EditorState;
class PluginProcessor;

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::ActionListener {
 public:
  PluginEditor(PluginProcessor& processor, Controller& controller,
               const EditorState& model,
               juce::AudioProcessorValueTreeState& parameters);
  ~PluginEditor() override;

  //============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

  void actionListenerCallback(const String& /*message*/) override;

 private:
  /// Controller.
  Controller& controller_;

  /// Model.
  const EditorState& model_;

  // [Control] -----------------------------------------------------------------
  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

  std::unique_ptr<juce::Slider> pitchBendSensitivitySlider_;
  std::unique_ptr<SliderAttachment> pitchBendSensitivitySliderAttachment_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
