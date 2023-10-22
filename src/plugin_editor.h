// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.h

#pragma once

#include <JuceHeader.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "action.h"
#include "apvts_attachment.h"
#include "audio/parameter/parameter.h"
#include "state.h"
#include "store.h"
#include "ui/attached_component.h"

class PluginProcessor;

namespace ui {
class EnvelopeGraph;
class FmOperatorParametersTabbedComponent;
}  // namespace ui

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor {
 public:
  /**
   * @brief Constructor.
   * @param[in] processor Audio plugin processor.
   * @param[in] store Store.
   * @param[in] parameters APVTS.
   */
  PluginEditor(PluginProcessor& processor,
               std::weak_ptr<PluginStore<PluginState, PluginAction>> store,
               juce::AudioProcessorValueTreeState& parameters);
  ~PluginEditor() override;

  //============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

 private:
  // Store.
  std::weak_ptr<PluginStore<PluginState, PluginAction>> store_;

  // [Control] -----------------------------------------------------------------
  // Label and slider pairs.
  std::unique_ptr<ui::LabeledSliderWithAttachment> pitchBendSensitivityPair_;
  std::unique_ptr<ui::LabeledSliderWithAttachment> alPair_, fbPair_;

  // Attachments for APVTS.
  std::vector<std::unique_ptr<ApvtsAttachment>> apvtsAttachments_;
  std::vector<std::unique_ptr<ApvtsAttachmentForUi>> apvtsUiAttachments_;

  // Tabbed component of FM operator parameters.
  std::unique_ptr<ui::FmOperatorParametersTabbedComponent> fmOperatorParamsTab_;

  // Envelope graph/
  std::shared_ptr<ui::EnvelopeGraph> envelopeGraph_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
