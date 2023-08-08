// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginProcessor.h

#pragma once

#include <JuceHeader.h>

#include <memory>

/**
 * @brief Utilities for plugin parameter.
 *
 */
namespace plugin_parameter {
/// Parameter type.
enum class Type {
  PitchBendSensitivity,
};

/**
 * @brief Get parameter identifier.
 *
 * @param[in] type Parameter type.
 * @return Parameter identifier.
 */
juce::ParameterID id(Type type);

/**
 * @brief Get paraemter identifier as \c juce::String.
 *
 * @param[in] type parameter type.
 * @return Parameter identifier text.
 */
juce::String idAsString(Type type);

/**
 * @brief Get parameter name.
 *
 * @param[in] type Parameter type.
 * @return Parameter name text.
 */
juce::String name(Type type);
}  // namespace plugin_parameter

//==============================================================================
class Controller;
class Model;

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

 private:
  //============================================================================

  /// Model for view state.
  std::shared_ptr<Model> model_;

  /// Parameters of plugin.
  juce::AudioProcessorValueTreeState parameters_;

  /// Controller.
  std::shared_ptr<Controller> controller_;

  /// Audio source.
  std::unique_ptr<audio::FmAudioSource> audioSource_;

  /// Resampler.
  std::unique_ptr<juce::ResamplingAudioSource> resampler_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
