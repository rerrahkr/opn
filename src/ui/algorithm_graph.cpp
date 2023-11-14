// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "algorithm_graph.h"

#include <tuple>

#include "../audio/parameter/parameter.h"
#include "colour.h"

namespace ui {
namespace {
const juce::Rectangle<float> kMaxSvgSize{0.f, 0.f, 452.f, 232.f};

/**
 * @brief Pairs of Svg binary data and its size for algorithm graph.
 *
 * The maximum size of svg bounding box is 452 px x 232 px.
 */
const std::pair<const char*, int>
    kAlgorithmSvgs[audio::parameter::AlgorithmValue::kMaximum -
                   audio::parameter::AlgorithmValue::kMinimum + 1]{
        std::make_pair(BinaryData::algorithm1_svg,
                       BinaryData::algorithm1_svgSize),
        std::make_pair(BinaryData::algorithm2_svg,
                       BinaryData::algorithm2_svgSize),
        std::make_pair(BinaryData::algorithm3_svg,
                       BinaryData::algorithm3_svgSize),
        std::make_pair(BinaryData::algorithm4_svg,
                       BinaryData::algorithm4_svgSize),
        std::make_pair(BinaryData::algorithm5_svg,
                       BinaryData::algorithm5_svgSize),
        std::make_pair(BinaryData::algorithm6_svg,
                       BinaryData::algorithm6_svgSize),
        std::make_pair(BinaryData::algorithm7_svg,
                       BinaryData::algorithm7_svgSize),
        std::make_pair(BinaryData::algorithm8_svg,
                       BinaryData::algorithm8_svgSize)};
}  // namespace

AlgorithmGraph::AlgorithmGraph(juce::AudioProcessorValueTreeState& parameters)
    : parameters_(parameters) {
  update();
}

void AlgorithmGraph::update() {
  removeAllChildren();

  const auto al =
      audio::parameter::parameterCast<audio::parameter::AlgorithmValue>(
          parameters_
              .getRawParameterValue(audio::parameter::idAsString(
                  audio::parameter::FmToneParameter::Al))
              ->load());
  const auto index = al.rawValue() - audio::parameter::AlgorithmValue::kMinimum;
  svg_ = std::apply(juce::Drawable::createFromImageData, kAlgorithmSvgs[index]);

  for (std::size_t slot = 0; slot < audio::kSlotCount; ++slot) {
    svg_->replaceColour(
        colour::graph::algorithm::kSvgStrokeSlot[slot],
        colour::graph::kSlot[slot].withAlpha(colour::graph::kSlotStrokeAlpha));
    svg_->replaceColour(
        colour::graph::algorithm::kSvgFillSlot[slot],
        colour::graph::kSlot[slot].withAlpha(colour::graph::kSlotFillAlpha));
  }
  svg_->replaceColour(colour::graph::algorithm::kSvgStrokeOut,
                      colour::graph::algorithm::kOut.withAlpha(
                          colour::graph::kSlotStrokeAlpha));
  svg_->replaceColour(
      colour::graph::algorithm::kSvgFillOut,
      colour::graph::algorithm::kOut.withAlpha(colour::graph::kSlotFillAlpha));

  addAndMakeVisible(svg_.get());

  resized();
}

void AlgorithmGraph::paint(juce::Graphics& graphics) {
  graphics.fillAll(colour::graph::kBackground);
}

void AlgorithmGraph::resized() {
  if (!svg_) {
    return;
  }

  constexpr int padding{10};
  const auto localBounds = getLocalBounds().reduced(padding).toFloat();

  const auto widthProportion = localBounds.getWidth() / kMaxSvgSize.getWidth();
  const auto heightProportion =
      localBounds.getHeight() / kMaxSvgSize.getHeight();
  const auto scale = juce::jmin(widthProportion, heightProportion);

  const auto originalSvgBounds = svg_->getDrawableBounds();
  const auto scaledViewBox =
      originalSvgBounds.transformedBy(juce::AffineTransform::scale(scale))
          .withCentre(localBounds.getCentre());
  svg_->setTransformToFit(scaledViewBox, juce::RectanglePlacement::centred);
}
}  // namespace ui
