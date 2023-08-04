// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.cpp

#include "plugin_editor.h"

#include "plugin_processor.h"

//==============================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processor_(p) {
  setSize(400, 300);
  resized();
}

PluginEditor::~PluginEditor() {}

//==============================================================================
void PluginEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}
