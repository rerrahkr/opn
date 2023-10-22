// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <functional>

/**
 * @brief Attachment for a parameter of @c juce::AudioProcessorValueTreeState.
 * @tparam ExecuteCallbackInMessageThread Whether callback for parameter change
 * should be executed in the message thread.
 */
template <bool ExecuteCallbackInMessageThread>
class AudioParameterValueTreeStateAttachment
    : public juce::AudioProcessorValueTreeState::Listener {
 public:
  /**
   * @brief Constructor.
   * @param[in] parameters Value tree.
   * @param[in] parameterId ID of attached parameter.
   * @param[in] callback Function called when the value of attached parameter is
   * changed.
   */
  AudioParameterValueTreeStateAttachment(
      juce::AudioProcessorValueTreeState& parameters,
      const juce::String& parameterId,
      const std::function<void(float)>& callback)
      : parameters_(parameters),
        parameterId_(parameterId),
        callback_(callback) {
    parameters.addParameterListener(parameterId, this);
  }

  ~AudioParameterValueTreeStateAttachment() {
    parameters_.removeParameterListener(parameterId_, this);
  }

  void parameterChanged(const String& /*parameterID*/,
                        float newValue) override {
    if constexpr (ExecuteCallbackInMessageThread) {
      // TODO: Prefer using juce::ChangerBroadCaster to get juce::MessageManager
      // for frequent updates.
      auto* messageManager = juce::MessageManager::getInstanceWithoutCreating();
      if (messageManager && callback_) {
        if (messageManager->isThisTheMessageThread()) {
          callback_(newValue);
        } else {
          juce::MessageManager::callAsync(
              [newValue, f = callback_] { f(newValue); });
        }
      }
    } else {
      if (callback_) {
        callback_(newValue);
      }
    }
  }

 private:
  juce::AudioProcessorValueTreeState& parameters_;
  const juce::String parameterId_;
  const std::function<void(float)> callback_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      AudioParameterValueTreeStateAttachment)
};

/// Attachment for APVTS.
using ApvtsAttachment = AudioParameterValueTreeStateAttachment<false>;

/// Attachment for APVTS which execute callback in message thread.
using ApvtsAttachmentForUi = AudioParameterValueTreeStateAttachment<true>;
