// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "controller.h"

#include "./ui/view_message.h"
#include "model.h"
#include "plugin_processor.h"

Controller::Controller(std::weak_ptr<Model> model, PluginProcessor& processor)
    : model_(std::move(model)), processor_(processor) {}

void Controller::handleMessage(const juce::Message& /*message*/) {
  auto model = model_.lock();
  if (!model) {
    return;
  }
}
