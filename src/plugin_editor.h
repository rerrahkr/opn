// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.h

#pragma once

#include <JuceHeader.h>

#include <memory>
#include <unordered_map>

#include "action.h"
#include "audio/parameter/parameter.h"
#include "state.h"
#include "store.h"
#include "ui/attached_component.h"

class PluginProcessor;

namespace ui {
class EnvelopeGraph;
}

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor {
 public:
  PluginEditor(PluginProcessor& processor,
               std::weak_ptr<PluginStore<PluginState, PluginAction>> store,
               juce::AudioProcessorValueTreeState& parameters);
  ~PluginEditor() override;

  //============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

 private:
  std::weak_ptr<PluginStore<PluginState, PluginAction>> store_;

  // [Control] -----------------------------------------------------------------
  std::unique_ptr<juce::Label> fbLabel_;
  std::unique_ptr<ui::AttachedSlider> fbSlider_;

  std::unique_ptr<ui::AttachedSlider> alSlider_;

  std::unordered_map<audio::parameter::FmOperatorParameter, ui::AttachedSlider>
      operatorSliders_[audio::kSlotCount];

  std::unique_ptr<ui::AttachedToggleButton>
      operatorEnabledButtons_[audio::kSlotCount];

  std::unique_ptr<juce::Label> pitchBendSensitivityLabel_;
  std::unique_ptr<ui::AttachedSlider> pitchBendSensitivitySlider_;

  std::shared_ptr<juce::ToggleButton>
      frontEnvelopeGraphChoiceButtons_[audio::kSlotCount];
  std::shared_ptr<ui::EnvelopeGraph> envelopeGraph_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
