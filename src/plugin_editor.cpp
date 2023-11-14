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
#include "ui/algorithm_graph.h"
#include "ui/colour.h"
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

  // Algorithm graph.
  algorithmGraph_ = std::make_shared<ui::AlgorithmGraph>(parameters);
  addAndMakeVisible(algorithmGraph_.get());

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

  // Panic button.
  panicButton_ = std::make_unique<juce::TextButton>("Panic!");
  panicButton_->onClick = [&processor] { processor.resetAudioSource(); };
  addAndMakeVisible(panicButton_.get());

  // Pitch bend sensitivity.
  pitchBendSensitivityPair_ = makeLabeledSlider(
      ap::idAsString(ap::PluginParameter::PitchBendSensitivity),
      "Pitch Bend Sensitivity", juce::Slider::IncDecButtons,
      juce::Slider::TextBoxLeft);

  // Algorithm.
  alPair_ =
      makeLabeledSlider(ap::idAsString(ap::FmToneParameter::Al), "Algorithm",
                        juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft);
  apvtsUiAttachments_.emplace_back(std::make_unique<ApvtsAttachmentForUi>(
      parameters, ap::idAsString(ap::FmToneParameter::Al),
      [weakGraph = std::weak_ptr(algorithmGraph_)](float /*newValue*/) {
        if (auto graph = weakGraph.lock()) {
          graph->update();
        }
      }));

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
    auto&& content =
        std::make_unique<ui::FmOperatorParametersTabContent>(slot, parameters);
    fmOperatorParamsTab_->addTab("Op." + juce::String(slot + 1u),
                                 ui::colour::tab::kSlot[slot],
                                 content.release(), true);
  }
  addAndMakeVisible(fmOperatorParamsTab_.get());

  setSize(700, 400);
  setResizeLimits(700, 400, std::numeric_limits<int>::max(),
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
    const auto paramsArea = leftArea.removeFromTop(kRowHeight * 3);
    ui::NestableGrid paramsGrid;
    paramsGrid.setTemplateColumns({1_fr, 1_fr});
    paramsGrid.setTemplateRows({1_fr, 1_fr, 1_fr});
    paramsGrid.setItems({pitchBendSensitivityPair_->label.get(),
                         pitchBendSensitivityPair_->slider.get(),
                         alPair_->label.get(), alPair_->slider.get(),
                         fbPair_->label.get(), fbPair_->slider.get()});
    paramsGrid.performLayout(paramsArea);
  }

  {
    const auto fmOperatorParamsTabArea =
        leftArea.removeFromTop(kRowHeight * 11);
    fmOperatorParamsTab_->setBounds(fmOperatorParamsTabArea);
  }

  {
    const auto buttonArea = leftArea.removeFromTop(kRowHeight);
    ui::NestableGrid buttonGrid;
    buttonGrid.setTemplateColumns({1_fr, 1_fr});
    buttonGrid.setTemplateRows({1_fr});

    buttonGrid.setItems({panicButton_.get(), {}});
    buttonGrid.performLayout(buttonArea);
  }

  // Right area.
  {
    constexpr int gap{kRowHeight / 2};
    const auto algorithmArea =
        rightArea.removeFromTop(kRowHeight * 7 + gap).withTrimmedBottom(gap);
    algorithmGraph_->setBounds(algorithmArea);
  }

  envelopeGraph_->setBounds(rightArea);
}
