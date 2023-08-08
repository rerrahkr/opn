// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <memory>

class Model;
class PluginProcessor;

class Controller : private juce::MessageListener {
 public:
  /**
   * @brief Constructor.
   * @param[in] model Model object.
   */
  explicit Controller(std::weak_ptr<Model> model, PluginProcessor& processor);

  using juce::MessageListener::postMessage;

 private:
  std::weak_ptr<Model> model_;
  PluginProcessor& processor_;

  /**
   * @brief Handle message from the view.
   * @param[in] message Mesage from the view.
   */
  void handleMessage(const juce::Message& message) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller);
};
