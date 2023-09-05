// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.h

#pragma once

#include <JuceHeader.h>

#include <memory>

#include "ui/attached_component.h"

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
  std::unique_ptr<juce::Label> fbLabel_;
  std::unique_ptr<ui::AttachedSlider> fbSlider_;

  std::unique_ptr<ui::AttachedSlider> alSlider_;

  struct OperatorControls {
    std::unique_ptr<ui::AttachedSlider> arSlider_, tlSlider_, drSlider_,
        slSlider_, srSlider_, rrSlider_, ksSlider_;
    std::unique_ptr<ui::AttachedSlider> mlSlider_, dtSlider_;
  };

  std::unique_ptr<juce::Label> pitchBendSensitivityLabel_;
  std::unique_ptr<ui::AttachedSlider> pitchBendSensitivitySlider_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
