// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginProcessor.h

#pragma once

#include <JuceHeader.h>

#include <memory>
#include <mutex>
#include <vector>

#include "action.h"
#include "apvts_attachment.h"
#include "audio/parameter/parameter.h"
#include "audio/parameter/parameter_change_queue.h"
#include "state.h"
#include "store.h"

namespace audio {
class FmAudioSource;
}

class PluginProcessor : public juce::AudioProcessor {
 public:
  //============================================================================
  PluginProcessor();
  ~PluginProcessor() override;

  //============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>& buffer,
                    juce::MidiBuffer& midiMessages) override;
  using AudioProcessor::processBlock;

  //============================================================================
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  //============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  //============================================================================
  /**
   * @brief Reserve parameter change.
   * @param[in] parameter Parameter change data.
   */
  void reserveParameterChange(
      const audio::parameter::ParameterVariant& parameter);

 private:
  //============================================================================

  /// Redux store.
  std::shared_ptr<PluginStore<PluginState, PluginAction>> store_;

  /// Parameters of plugin.
  juce::AudioProcessorValueTreeState parameters_;

  /// Attachments for APVTS.
  std::vector<std::unique_ptr<ApvtsAttachment>> attachments_;

  /// Audio source.
  std::unique_ptr<audio::FmAudioSource> audioSource_;

  /// Resampler.
  std::unique_ptr<juce::ResamplingAudioSource> resampler_;

  std::mutex parameterQueueMutex_;

  /// Queue storing notifications of parameter change.
  audio::parameter::ParameterChangeQueue parameterChangeQueue_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
