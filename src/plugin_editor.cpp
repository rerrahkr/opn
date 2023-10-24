// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: 2023 Rerrah
// Original source comes from JUCE Git repository:
// JUCE/examples/CMake/AudioPlugin/PluginEditor.cpp

#include "plugin_editor.h"

#include <limits>
#include <type_traits>
#include <utility>

#include "apvts_attachment.h"
#include "plugin_processor.h"
#include "ui/envelope_graph.h"
#include "ui/fm_operator_parameters_tab_content.h"
#include "ui/fm_operator_parameters_tabbed_component.h"
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

  namespace ap = audio::parameter;

  const auto makeLabeledSlider = [&](const juce::String& parameterId,
                                     const juce::String& labelText,
                                     auto&&... sliderArgs) {
    auto&& labeledSlider = std::make_unique<ui::LabeledSliderWithAttachment>(
        parameters, parameterId, labelText,
        std::forward<decltype(sliderArgs)>(sliderArgs)...);
    addAndMakeVisible(labeledSlider->label.get());
    addAndMakeVisible(labeledSlider->slider.get());
    return labeledSlider;
  };

  // Pitch bend sensitivity.
  pitchBendSensitivityPair_ = makeLabeledSlider(
      ap::idAsString(ap::PluginParameter::PitchBendSensitivity),
      "Pitch Bend Sensitivity", juce::Slider::IncDecButtons,
      juce::Slider::TextBoxLeft);

  // Algorithm.
  alPair_ =
      makeLabeledSlider(ap::idAsString(ap::FmToneParameter::Al), "Algorithm",
                        juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft);

  // Feedback.
  fbPair_ = makeLabeledSlider(ap::idAsString(ap::FmToneParameter::Fb),
                              "Feedback", juce::Slider::LinearHorizontal,
                              juce::Slider::TextBoxRight);

  // Attach callbacks for operator parameters.
  for (std::size_t slot = 0; slot < audio::kSlotCount; ++slot) {
    // Callback for UI.
    const auto attachCallbackForUi =
        [&](ap::FmOperatorParameter parameterType) {
          apvtsUiAttachments_.emplace_back(
              std::make_unique<ApvtsAttachmentForUi>(
                  parameters, ap::idAsString(slot, parameterType),
                  [weakGraph =
                       std::weak_ptr(envelopeGraph_)](float /*newValue*/) {
                    if (auto graph = weakGraph.lock()) {
                      graph->updateControllerPosition();
                    }
                  }));
        };

    attachCallbackForUi(ap::FmOperatorParameter::OperatorEnabled);
    attachCallbackForUi(ap::FmOperatorParameter::Ar);
    attachCallbackForUi(ap::FmOperatorParameter::Dr);
    attachCallbackForUi(ap::FmOperatorParameter::Sr);
    attachCallbackForUi(ap::FmOperatorParameter::Rr);
    attachCallbackForUi(ap::FmOperatorParameter::Sl);
    attachCallbackForUi(ap::FmOperatorParameter::Tl);
  }

  fmOperatorParamsTab_ =
      std::make_unique<ui::FmOperatorParametersTabbedComponent>(
          juce::TabbedButtonBar::TabsAtTop, [store](int tabIndex) {
            if (auto storePtr = store.lock()) {
              storePtr->dispatch(PluginAction{
                  .type{PluginAction::Type::CurrentEditingOperatorChanged},
                  .payload{static_cast<std::size_t>(tabIndex)}});
            }
          });
  for (std::size_t slot = 0; slot < audio::kSlotCount; ++slot) {
    constexpr auto kLightness{.3f}, kAlpha{1.f};
    static const juce::Colour backColours[]{
        juce::Colour::fromHSL(.42f, .30f, kLightness, kAlpha),
        juce::Colour::fromHSL(.83f, .30f, kLightness, kAlpha),
        juce::Colour::fromHSL(.60f, .30f, kLightness, kAlpha),
        juce::Colour::fromHSL(.13f, .30f, kLightness, kAlpha)};
    auto&& content =
        std::make_unique<ui::FmOperatorParametersTabContent>(slot, parameters);
    fmOperatorParamsTab_->addTab("Op." + juce::String(slot + 1u),
                                 backColours[slot], content.release(), true);
  }
  addAndMakeVisible(fmOperatorParamsTab_.get());

  setSize(800, 400);
  setResizeLimits(600, 400, std::numeric_limits<int>::max(),
                  std::numeric_limits<int>::max());
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
  constexpr int kContentAreaPadding{20};
  constexpr int kLeftAreaWidth{300};
  constexpr int kRowHeight{20};

  auto area = getLocalBounds().reduced(kContentAreaPadding);

  juce::Rectangle<int> leftArea, rightArea;
  ui::NestableGrid contentGrid;
  contentGrid.setTemplateColumns({juce::Grid::Px{kLeftAreaWidth}, 1_fr});
  contentGrid.setTemplateRows({1_fr});
  contentGrid.setItems(
      {ui::NestableGridItem{leftArea}, ui::NestableGridItem{rightArea}});
  contentGrid.setGap(juce::Grid::Px{kContentAreaPadding});
  contentGrid.performLayout(area);

  // Left area.
  {
    const auto pluginParamsArea = leftArea.removeFromTop(kRowHeight);
    ui::NestableGrid pluginParamsGrid;
    pluginParamsGrid.setTemplateColumns({1_fr, 1_fr});
    pluginParamsGrid.setTemplateRows({1_fr});
    pluginParamsGrid.setItems({pitchBendSensitivityPair_->label.get(),
                               pitchBendSensitivityPair_->slider.get()});
    pluginParamsGrid.performLayout(pluginParamsArea);
  }

  {
    const auto toneParamsArea = leftArea.removeFromTop(kRowHeight * 2);
    ui::NestableGrid toneParamsGrid;
    toneParamsGrid.setTemplateColumns({1_fr, 1_fr});
    toneParamsGrid.setTemplateRows({1_fr, 1_fr});
    toneParamsGrid.setItems({alPair_->label.get(), alPair_->slider.get(),
                             fbPair_->label.get(), fbPair_->slider.get()});
    toneParamsGrid.performLayout(toneParamsArea);
  }

  {
    const auto fmOperatorParamsTabArea =
        leftArea.removeFromTop(kRowHeight * 11);
    fmOperatorParamsTab_->setBounds(fmOperatorParamsTabArea);
  }

  // Right area.
  envelopeGraph_->setBounds(rightArea);
}
