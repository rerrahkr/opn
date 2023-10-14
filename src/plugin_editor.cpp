// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.cpp

#include "plugin_editor.h"

#include <utility>

#include "plugin_processor.h"
#include "ui/envelope_graph.h"
#include "ui/nestable_grid.h"

//==============================================================================
PluginEditor::PluginEditor(
    PluginProcessor& processor,
    std::weak_ptr<PluginStore<PluginState, PluginAction>> store,
    juce::AudioProcessorValueTreeState& parameters)
    : AudioProcessorEditor(&processor), store_(store) {
  envelopeGraph_ = std::make_shared<ui::EnvelopeGraph>(parameters);
  addAndMakeVisible(envelopeGraph_.get());
  if (auto storePtr = store.lock()) {
    storePtr->subscribe(
        [weakGraph = std::weak_ptr(envelopeGraph_)](const auto& state) {
          if (auto graph = weakGraph.lock()) {
            graph->render(state);
          }
        });
  }

  const auto makeLabel = [](const juce::String& text) {
    return std::make_unique<juce::Label>("", text);
  };

  namespace ap = audio::parameter;

  // Pitch bend sensitivity.
  pitchBendSensitivityLabel_ = makeLabel("Pitch bend sensitivity");
  addAndMakeVisible(pitchBendSensitivityLabel_.get());
  pitchBendSensitivitySlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      ap::idAsString(ap::PluginParameter::PitchBendSensitivity),
      [&processor](float newValue) {
        processor.reserveParameterChange(
            ap::parameterCast<ap::PitchBendSensitivityValue>(newValue));
      });
  addAndMakeVisible(pitchBendSensitivitySlider_->slider);

  // Feedback.
  fbLabel_ = makeLabel("Feedback");
  addAndMakeVisible(fbLabel_.get());
  fbSlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight, parameters,
      ap::idAsString(ap::FmToneParameter::Fb), [&processor](float newValue) {
        processor.reserveParameterChange(
            ap::parameterCast<ap::FeedbackValue>(newValue));
      });
  addAndMakeVisible(fbSlider_->slider);

  // Algorithm.
  alSlider_ = std::make_unique<ui::AttachedSlider>(
      juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft, parameters,
      ap::idAsString(ap::FmToneParameter::Al), [&processor](float newValue) {
        processor.reserveParameterChange(
            ap::parameterCast<ap::AlgorithmValue>(newValue));
      });
  addAndMakeVisible(alSlider_->slider);

  // Operator parameters.
  for (std::size_t i = 0; i < audio::kSlotCount; ++i) {
    auto enabledButton = std::make_unique<ui::AttachedToggleButton>(
        parameters, ap::idAsString(i, ap::FmOperatorParameter::OperatorEnabled),
        [&processor, i](float newValue) {
          processor.reserveParameterChange(ap::SlotAndValue(
              i, ap::parameterCast<ap::OperatorEnabledValue>(newValue)));
        });
    addAndMakeVisible(enabledButton->button);
    operatorEnabledButtons_[i] = std::move(enabledButton);

    // Slider initialization lambda for operator parameters.
    static const auto initialiseSlider =
        [&](std::size_t slot, ap::FmOperatorParameter parameterType,
            auto conversion) {
          [[maybe_unused]] auto [iter, _] = operatorSliders_[slot].emplace(
              std::piecewise_construct, std::forward_as_tuple(parameterType),
              std::forward_as_tuple(
                  juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight,
                  parameters, ap::idAsString(slot, parameterType),
                  [&processor, slot, conversion,
                   weakGraph = std::weak_ptr(envelopeGraph_)](float newValue) {
                    processor.reserveParameterChange(
                        ap::SlotAndValue{slot, conversion(newValue)});
                    if (auto graph = weakGraph.lock()) {
                      graph->updateControllerPosition();
                    }
                  }));
          addAndMakeVisible(iter->second.slider);
        };

    initialiseSlider(i, ap::FmOperatorParameter::Ar, [](float newValue) {
      return ap::parameterCast<ap::AttackRateValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Dr, [](float newValue) {
      return ap::parameterCast<ap::DecayRateValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Sr, [](float newValue) {
      return ap::parameterCast<ap::SustainRateValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Rr, [](float newValue) {
      return ap::parameterCast<ap::ReleaseRateValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Sl, [](float newValue) {
      return ap::parameterCast<ap::SustainLevelValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Tl, [](float newValue) {
      return ap::parameterCast<ap::AttackRateValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Ks, [](float newValue) {
      return ap::parameterCast<ap::KeyScaleValue>(newValue);
    });
    initialiseSlider(i, ap::FmOperatorParameter::Dt, [](float newValue) {
      return ap::parameterCast<ap::DetuneValue>(newValue);
    });
  }

  for (std::size_t slot = 0; slot < audio::kSlotCount; ++slot) {
    constexpr int radioGroupId{1};
    auto button =
        std::make_shared<juce::ToggleButton>("Op-" + juce::String(slot + 1u));
    button->setRadioGroupId(radioGroupId);
    button->onClick = [slot, store, weakButton = std::weak_ptr(button)] {
      if (auto button = weakButton.lock(); button && button->getToggleState()) {
        if (auto storePtr = store.lock()) {
          storePtr->dispatch(PluginAction{
              .type{PluginAction::Type::EnvelopeGraphFrontRadioButtonChanged},
              .payload{slot}});
        }
      }
    };
    addAndMakeVisible(button.get());
    frontEnvelopeGraphChoiceButtons_[slot] = std::move(button);
  }
  frontEnvelopeGraphChoiceButtons_[0]->triggerClick();

  setSize(1000, 500);
  setResizable(true, false);
  resized();
}

PluginEditor::~PluginEditor() {}

//==============================================================================
void PluginEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized() {
  constexpr int kRowHeight{20}, kSliderWidth{120}, kToggleButtonWidth{20};
  int y{};

  pitchBendSensitivityLabel_->setBounds(0, y, 200, kRowHeight);
  pitchBendSensitivitySlider_->slider.setBounds(200, y, kSliderWidth,
                                                kRowHeight);
  y += kRowHeight;

  fbLabel_->setBounds(0, y, 100, kRowHeight);
  fbSlider_->slider.setBounds(100, y, kSliderWidth, kRowHeight);
  y += kRowHeight;

  alSlider_->slider.setBounds(0, y, kSliderWidth, kRowHeight);
  y += kRowHeight;

  for (auto& sliderMap : operatorSliders_) {
    for ([[maybe_unused]] auto& [_, slider] : sliderMap) {
      slider.slider.setBounds(0, y, kSliderWidth + 100, kRowHeight);
      y += kRowHeight;
    }
  }

  for (auto& button : operatorEnabledButtons_) {
    button->button.setBounds(0, y, kToggleButtonWidth, kRowHeight);
    y += kRowHeight;
  }

  for (std::size_t slot = 0; slot < audio::kSlotCount; ++slot) {
    constexpr int width{100};
    frontEnvelopeGraphChoiceButtons_[slot]->setBounds(
        400 + width * static_cast<int>(slot), 0, width, kRowHeight);
  }

  envelopeGraph_->setBounds(400, kRowHeight, getRight() - 400,
                            getBottom() - kRowHeight);
}
