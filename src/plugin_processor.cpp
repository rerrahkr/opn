// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginProcessor.cpp

#include "plugin_processor.h"

#include <unordered_map>
#include <utility>

#include "audio/fm_audio_source.h"
#include "audio/pitch_util.h"
#include "controller.h"
#include "model.h"
#include "plugin_editor.h"

namespace {
/// Default pitch bend sensitivity.
constexpr std::uint8_t kDefaultPitchBendSensitivity{2};

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;
  audio::FmParameters defaultParameters;

  layout.add(std::make_unique<juce::AudioParameterInt>(
      audio::parameter::id(
          audio::parameter::PluginParameter::PitchBendSensitivity),
      audio::parameter::name(
          audio::parameter::PluginParameter::PitchBendSensitivity),
      1, audio::pitch_util::kMaxPitchBendSensitivity,
      kDefaultPitchBendSensitivity));

  return layout;
}
}  // namespace

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      model_(std::make_shared<Model>()),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout()),
      controller_(std::make_shared<Controller>(model_, *this)),
      audioSource_(std::make_unique<audio::FmAudioSource>()) {
}

PluginProcessor::~PluginProcessor() {}

//==============================================================================
const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const { return 0.0; }

int PluginProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int PluginProcessor::getCurrentProgram() { return 0; }

void PluginProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void PluginProcessor::changeProgramName(int index,
                                        const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  audioSource_->prepareToPlay(samplesPerBlock, sampleRate);

  resampler_ = std::make_unique<juce::ResamplingAudioSource>(
      audioSource_.get(), false, getMainBusNumOutputChannels());
  const auto synrate = audioSource_->synthesisRate();
  const auto ratio = synrate / sampleRate;
  resampler_->setResamplingRatio(ratio);
  resampler_->prepareToPlay(samplesPerBlock, sampleRate);
}

void PluginProcessor::releaseResources() {
  audioSource_->releaseResources();
  resampler_->releaseResources();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;

  buffer.clear(0, buffer.getNumSamples());

  {
    // Reflect parameter changes modified by sliders.
    std::lock_guard<std::mutex> guard(parameterQueueMutex_);

    while (!parameterChangeQueue_.empty()) {
      auto&& parameter = parameterChangeQueue_.dequeue();
      std::visit(audio::ParameterVisiter{*audioSource_, parameters_},
                 parameter);
    }
  }

  if (!resampler_) {
    return;
  }

  int sampleStartPosition{};

  // Fill buffer considering MIDI events.
  for (auto iter = midiMessages.begin(); iter != midiMessages.end();) {
    const int position = (*iter).samplePosition;

    // Try to change a state of audio source.
    if (!audioSource_->tryReserveChangeFromMidiMessage(
            (*iter++).getMessage())) {
      continue;
    }

    if (iter != midiMessages.end() && (*iter).samplePosition == position) {
      // Continue to read next event which is placed at the same position.
      continue;
    }

    juce::AudioSourceChannelInfo channelInfo(&buffer, sampleStartPosition,
                                             position - sampleStartPosition);
    resampler_->getNextAudioBlock(channelInfo);
    sampleStartPosition = position;

    audioSource_->triggerReservedChanges();
  }

  // Fill a rest of buffer.
  juce::AudioSourceChannelInfo channelInfo(
      &buffer, sampleStartPosition,
      buffer.getNumSamples() - sampleStartPosition);
  resampler_->getNextAudioBlock(channelInfo);
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
  return new PluginEditor(*this, *controller_, *model_, parameters_);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
  const auto state = parameters_.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (!xmlState || !xmlState->hasTagName(parameters_.state.getType())) {
    return;
  }

  parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new PluginProcessor();
}

//==============================================================================
void PluginProcessor::reserveParameterChange(audio::Parameter parameter) {
  std::lock_guard<std::mutex> guard(parameterQueueMutex_);
  parameterChangeQueue_.enqueue(parameter);
}
