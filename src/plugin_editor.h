// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.h

#pragma once

#include <memory>

#include "plugin_processor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor {
 public:
  explicit PluginEditor(PluginProcessor& p);
  ~PluginEditor() override;

  //==============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PluginProcessor& processor_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
