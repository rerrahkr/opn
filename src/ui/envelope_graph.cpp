// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "envelope_graph.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "../util.h"

namespace ui {
namespace {
namespace ap = audio::parameter;

namespace colour {
constexpr float kLineAlpha{.75f}, kNodeFillAlpha{.1f};
constexpr float kLineFrontLightness{.54f}, kLineBackLightness{.30f};
constexpr float kEnvelopeFillGradientOpacityMultiply{.2f};

const juce::Colour kLink[audio::kSlotCount]{
    juce::Colour::fromHSL(.42f, .70f, kLineFrontLightness, kLineAlpha),
    juce::Colour::fromHSL(.83f, .70f, kLineFrontLightness, kLineAlpha),
    juce::Colour::fromHSL(.60f, .70f, kLineFrontLightness, kLineAlpha),
    juce::Colour::fromHSL(.13f, .70f, kLineFrontLightness, kLineAlpha)};
const auto kBackground = juce::Colour::fromHSL(0.f, 0.f, .12f, 1.f);
const juce::Colour kNodeFill[audio::kSlotCount]{
    kLink[0].withAlpha(kNodeFillAlpha), kLink[1].withAlpha(kNodeFillAlpha),
    kLink[2].withAlpha(kNodeFillAlpha), kLink[3].withAlpha(kNodeFillAlpha)};
const juce::Colour kNodeBorder[audio::kSlotCount]{kLink[0], kLink[1], kLink[2],
                                                  kLink[3]};
}  // namespace colour

namespace stroke_thickness {
constexpr float kFrontLink{2.f};
constexpr float kBackLink{kFrontLink * .5f};
constexpr float kNode{kFrontLink};
}  // namespace stroke_thickness

/// Controller size by pixel.
constexpr int kControllerSize{16 /*px*/};

/// Affine transform operation to translate coordinate based on top-left
/// position.
const auto kControllerShift = juce::AffineTransform::translation(
    -kControllerSize * .5f, -kControllerSize * .5f);

/// Minimum value of rate parameters which changes amplitude.
constexpr int kMinSlopedAr{ap::AttackRateValue::kMinimum + 1},
    kMinSlopedDr{ap::DecayRateValue::kMinimum + 1},
    kMinSlopedSr{ap::SustainRateValue::kMinimum + 1},
    kMinSlopedRr{ap::ReleaseRateValue::kMinimum + 1};
}  // namespace

//==============================================================================

ControlPoint::ControlPoint(const juce::Colour& fillColour,
                           const juce::Colour& borderColour)
    : fillColour_(fillColour), borderColour_(borderColour) {
  setMouseCursor(juce::MouseCursor::DraggingHandCursor);
  resized();
}

bool ControlPoint::hitTest(int x, int y) {
  return hitTestShape_.contains(static_cast<float>(x), static_cast<float>(y));
}

void ControlPoint::paint(juce::Graphics& graphics) {
  graphics.setColour(fillColour_);
  graphics.fillPath(drawingShape_);

  graphics.setColour(borderColour_);
  graphics.strokePath(drawingShape_,
                      juce::PathStrokeType{stroke_thickness::kNode});
}

void ControlPoint::resized() {
  const auto localBounds = getLocalBounds().toFloat();

  hitTestShape_.clear();
  hitTestShape_.addEllipse(localBounds);

  drawingShape_.clear();
  drawingShape_.addEllipse(localBounds.reduced(stroke_thickness::kNode));
}

//==============================================================================

EnvelopeGraph::EnvelopeGraph(juce::AudioProcessorValueTreeState& parameters)
    : parameters_(parameters) {
  for (std::size_t slot = 0u; slot < audio::kSlotCount; ++slot) {
    for (std::size_t i : {3u, 2u, 1u, 0u}) {
      auto controller = std::make_unique<ControlPoint>(
          colour::kNodeFill[slot], colour::kNodeBorder[slot]);
      controller->setSize(kControllerSize, kControllerSize);
      addChildComponent(controller.get());
      controllers_[slot][i] = std::move(controller);
    }
  }

  addMouseListener(this, true);

  resized();
}

void EnvelopeGraph::mouseDown(const juce::MouseEvent& event) {
  const auto& eventComponent = event.eventComponent;

  if (std::any_of(visibleControllers_, visibleControllers_ + kControllerCount_,
                  [eventComponent](const auto& controller) {
                    return controller.get() == eventComponent;
                  })) {
    controllerTopLeftPositionAtDragStart_ = eventComponent->getPosition();
  }
}

void EnvelopeGraph::mouseDrag(const juce::MouseEvent& event) {
  const auto& eventComponent = event.eventComponent;

  if (controllerTopLeftPositionAtDragStart_) {
    const auto draggingTopLeftPosition =
        controllerTopLeftPositionAtDragStart_.value() +
        event.getOffsetFromDragStart();

    if (eventComponent == visibleControllers_[0].get()) {
      onController1Dragged(draggingTopLeftPosition);
    } else if (eventComponent == visibleControllers_[1].get()) {
      onController2Dragged(draggingTopLeftPosition);
    } else if (eventComponent == visibleControllers_[2].get()) {
      onController3Dragged(draggingTopLeftPosition);
    } else if (eventComponent == visibleControllers_[3].get()) {
      onController4Dragged(draggingTopLeftPosition);
    }

    repaint();
  } else {
    juce::Component::mouseDrag(event);
  }
}

void EnvelopeGraph::mouseUp(const juce::MouseEvent& /* event */) {
  controllerTopLeftPositionAtDragStart_.reset();
}

void EnvelopeGraph::onController1Dragged(
    const juce::Point<int>& draggingTopLeftPosition) {
  const auto draggableTopLeftArea =
      getLocalBounds().withWidth(kMaxArWidth).transformedBy(kControllerShift);
  const auto clippedTopLeftPosition =
      draggableTopLeftArea.getConstrainedPoint(draggingTopLeftPosition);

  const auto tl = juce::roundToInt(juce::jmap<double>(
      clippedTopLeftPosition.y, draggableTopLeftArea.getY(),
      draggableTopLeftArea.getBottom(), ap::TotalLevelValue::kMinimum,
      ap::TotalLevelValue::kMaximum));

  const auto ar = [&]() -> int {
    if (tl == ap::TotalLevelValue::kMaximum) {
      return ap::AttackRateValue::kMinimum;
    } else {
      return juce::roundToInt(juce::jmap<double>(
          clippedTopLeftPosition.x, draggableTopLeftArea.getX(),
          draggableTopLeftArea.getRight(), ap::AttackRateValue::kMaximum,
          kMinSlopedAr));
    }
  }();

  for (const auto& [id, value] :
       {std::make_pair(frontArId_, ar), std::make_pair(frontTlId_, tl)}) {
    auto* parameter = parameters_.getParameter(id);
    const auto normalisedValue =
        parameter->convertTo0to1(static_cast<float>(value));
    if (normalisedValue == parameter->getValue()) {
      continue;
    }

    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(normalisedValue);
    parameter->endChangeGesture();
  }
}

void EnvelopeGraph::onController2Dragged(
    const juce::Point<int>& draggingTopLeftPosition) {
  const auto draggableTopLeftArea = [this] {
    const auto controller1CentrePosition =
        visibleControllers_[0]->getBounds().getCentre();
    return juce::Rectangle{controller1CentrePosition.x,
                           controller1CentrePosition.y, kMaxDrSrWidth,
                           getHeight() - controller1CentrePosition.y}
        .transformedBy(kControllerShift);
  }();
  if (!draggableTopLeftArea.getHeight()) {
    // Cannot drag controller 2 if controller 1 is placed at the bottom of
    // graph area.
    return;
  }

  const auto clippedTopLeftPosition =
      draggableTopLeftArea.getConstrainedPoint(draggingTopLeftPosition);

  const auto&& [sl, topPosition] = util::quantize<int>(
      clippedTopLeftPosition.y, draggableTopLeftArea.getY(),
      draggableTopLeftArea.getBottom(), ap::SustainLevelValue::kMinimum,
      ap::SustainLevelValue::kMaximum);

  const auto dr = [&]() -> int {
    if (sl == ap::SustainLevelValue::kMinimum) {
      if (clippedTopLeftPosition.x == draggableTopLeftArea.getX()) {
        // Controller 2 is placed at the top-left corner.
        return ap::DecayRateValue::kMaximum;
      } else {
        // Controller 2 is placed on the top line.
        return ap::DecayRateValue::kMinimum;
      }
    } else {
      // Calculate DR.
      const auto scaleFactor =
          static_cast<float>(topPosition - draggableTopLeftArea.getY()) /
          draggableTopLeftArea.getHeight();
      const auto&& scaledArea =
          draggableTopLeftArea.transformedBy(juce::AffineTransform::scale(
              scaleFactor, scaleFactor,
              static_cast<float>(draggableTopLeftArea.getX()),
              static_cast<float>(draggableTopLeftArea.getY())));
      return juce::roundToInt(juce::jmap<double>(
          juce::jmin(clippedTopLeftPosition.x, scaledArea.getRight()),
          scaledArea.getX(), scaledArea.getRight(),
          ap::DecayRateValue::kMaximum, kMinSlopedDr));
    }
  }();

  for (const auto& [id, value] :
       {std::make_pair(frontDrId_, dr), std::make_pair(frontSlId_, sl)}) {
    auto* parameter = parameters_.getParameter(id);
    const auto normalisedValue =
        parameter->convertTo0to1(static_cast<float>(value));
    if (normalisedValue == parameter->getValue()) {
      continue;
    }

    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(normalisedValue);
    parameter->endChangeGesture();
  }
}

void EnvelopeGraph::onController3Dragged(
    const juce::Point<int>& draggingTopLeftPosition) {
  const auto& controller1 = visibleControllers_[0];
  const auto& controller2 = visibleControllers_[1];

  if (controller2->getBounds().getCentreY() == getLocalBounds().getBottom()) {
    // Cannot drag controller 3 if controller 1 is placed at the bottom of graph
    // area.
    return;
  } else if (controller2->getX() != controller1->getX() &&
             controller2->getY() == controller1->getY()) {
    // Cannot drag controller 3 if controller 2 is placed at the right of
    // draggable area.
    return;
  }

  const auto draggableTopLeftArea = [&] {
    const auto controller1CentrePosition = controller1->getBounds().getCentre();
    const auto controller2CentrePosition = controller2->getBounds().getCentre();
    const auto controller2DraggablePosition =
        getLocalBounds()
            .withTrimmedLeft(controller1CentrePosition.x)
            .withTrimmedTop(controller1CentrePosition.y)
            .withWidth(kMaxDrSrWidth);
    const auto scale =
        .5f * (getLocalBounds().getBottom() - controller2CentrePosition.y) /
        controller2DraggablePosition.getHeight();
    return controller2DraggablePosition.withPosition(controller2CentrePosition)
        .transformedBy(juce::AffineTransform::scale(
            scale, scale, static_cast<float>(controller2CentrePosition.x),
            static_cast<float>(controller2CentrePosition.y)))
        .transformedBy(kControllerShift);
  }();

  const auto clippedTopLeftPosition =
      draggableTopLeftArea.getConstrainedPoint(draggingTopLeftPosition);

  const auto sr = [&]() -> int {
    if (clippedTopLeftPosition.y == draggableTopLeftArea.getY()) {
      return ap::SustainRateValue::kMinimum;
    } else {
      return juce::roundToInt(juce::jmap<double>(
          clippedTopLeftPosition.x, draggableTopLeftArea.getX(),
          draggableTopLeftArea.getRight(), ap::SustainRateValue::kMaximum,
          kMinSlopedSr));
    }
  }();

  auto* parameter = parameters_.getParameter(frontSrId_);
  const auto normalisedValue = parameter->convertTo0to1(static_cast<float>(sr));
  if (normalisedValue != parameter->getValue()) {
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(normalisedValue);
    parameter->endChangeGesture();
  }
}

void EnvelopeGraph::onController4Dragged(
    const juce::Point<int>& draggingTopLeftPosition) {
  const auto draggableTopLeftArea = [&] {
    const auto controller3CentrePosition =
        visibleControllers_[2]->getBounds().getCentre();
    return juce::Rectangle<int>{
        controller3CentrePosition.x, controller3CentrePosition.y, kMaxRrWidth,
        getLocalBounds().getBottom() - controller3CentrePosition.y}
        .transformedBy(kControllerShift);
  }();
  if (!draggableTopLeftArea.getHeight()) {
    // Cannot drag controller 4 if controller 1 or 2 are placed at the bottom of
    // the graph area.
    return;
  }

  const auto clippedTopLeftPosition =
      draggableTopLeftArea.getConstrainedPoint(draggingTopLeftPosition);

  const auto rr = [&]() -> int {
    if (clippedTopLeftPosition.y == draggableTopLeftArea.getY()) {
      return ap::ReleaseRateValue::kMinimum;
    } else {
      return juce::roundToInt(juce::jmap<double>(
          clippedTopLeftPosition.x, draggableTopLeftArea.getX(),
          draggableTopLeftArea.getRight(), ap::ReleaseRateValue::kMaximum,
          kMinSlopedRr));
    }
  }();

  auto* parameter = parameters_.getParameter(frontRrId_);
  const auto normalisedValue = parameter->convertTo0to1(static_cast<float>(rr));
  if (normalisedValue != parameter->getValue()) {
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(normalisedValue);
    parameter->endChangeGesture();
  }
}

void EnvelopeGraph::updateControllerPosition() {
  const auto topLeftBounds = getLocalBounds().transformedBy(kControllerShift);

  for (std::size_t slot = 0u; slot < audio::kSlotCount; ++slot) {
    updateTopLeftPositionOfController1(slot, topLeftBounds);
    updateTopLeftPositionOfController2(slot, topLeftBounds);
    updateTopLeftPositionOfController3(slot, topLeftBounds);
    updateTopLeftPositionOfController4(slot, topLeftBounds);
  }

  repaint();
}

void EnvelopeGraph::updateTopLeftPositionOfController1(
    std::size_t slot, const juce::Rectangle<int>& topLeftBounds) {
  auto& controller1 = controllers_[slot][0];

  const auto tl = juce::roundToInt(parameters_
                                       .getRawParameterValue(ap::idAsString(
                                           slot, ap::FmOperatorParameter::Tl))
                                       ->load());
  if (tl == ap::TotalLevelValue::kMaximum) {
    // Crush envelope.
    controller1->setTopLeftPosition(topLeftBounds.getBottomLeft());
    return;
  }

  const auto top = juce::jmap<int>(
      tl, ap::TotalLevelValue::kMinimum, ap::TotalLevelValue::kMaximum,
      topLeftBounds.getY(), topLeftBounds.getBottom());

  const auto ar = juce::roundToInt(parameters_
                                       .getRawParameterValue(ap::idAsString(
                                           slot, ap::FmOperatorParameter::Ar))
                                       ->load());
  if (ar == ap::AttackRateValue::kMinimum) {
    // Crush envelope.
    controller1->setTopLeftPosition(topLeftBounds.getBottomLeft());
    return;
  }

  const auto left =
      juce::jmap<int>(ar, ap::AttackRateValue::kMaximum, kMinSlopedAr,
                      topLeftBounds.getX(), kMaxArWidth);

  controller1->setTopLeftPosition(left, top);
}

void EnvelopeGraph::updateTopLeftPositionOfController2(
    std::size_t slot, const juce::Rectangle<int>& topLeftBounds) {
  const auto& controller1 = controllers_[slot][0];
  auto& controller2 = controllers_[slot][1];

  // Calculate position of controller 2 on the extention line of L2.
  const juce::Rectangle<int> movableTopLeftArea{
      controller1->getX(), controller1->getY(), kMaxDrSrWidth,
      topLeftBounds.getBottom() - controller1->getY()};
  if (!movableTopLeftArea.getHeight()) {
    // Placed at the same position of controller 1, which is placed at the
    // bottom of the graph.
    controller2->setTopLeftPosition(controller1->getPosition());
    return;
  }

  const auto sl = juce::roundToInt(parameters_
                                       .getRawParameterValue(ap::idAsString(
                                           slot, ap::FmOperatorParameter::Sl))
                                       ->load());
  const auto dr = juce::roundToInt(parameters_
                                       .getRawParameterValue(ap::idAsString(
                                           slot, ap::FmOperatorParameter::Dr))
                                       ->load());

  if (sl == ap::SustainLevelValue::kMinimum) {
    if (dr == ap::DecayRateValue::kMaximum) {
      // Placed at the top-left corner.
      controller2->setTopLeftPosition(controller1->getPosition());
    } else {
      // Placed on the top line.
      controller2->setTopLeftPosition(
          controller1->getPosition().translated(kMaxDrSrWidth, 0));
    }
    return;
  }

  const auto top = juce::jmap<int>(
      sl, ap::SustainLevelValue::kMinimum, ap::SustainLevelValue::kMaximum,
      movableTopLeftArea.getY(), movableTopLeftArea.getBottom());

  const auto scaleFactor = static_cast<float>(top - movableTopLeftArea.getY()) /
                           movableTopLeftArea.getHeight();
  const auto&& scaledArea =
      movableTopLeftArea.transformedBy(juce::AffineTransform::scale(
          scaleFactor, scaleFactor,
          static_cast<float>(movableTopLeftArea.getX()),
          static_cast<float>(movableTopLeftArea.getY())));

  const auto left =
      juce::jmap<int>(dr, ap::DecayRateValue::kMaximum, kMinSlopedDr,
                      scaledArea.getX(), scaledArea.getRight());

  controller2->setTopLeftPosition(left, top);
}

void EnvelopeGraph::updateTopLeftPositionOfController3(
    std::size_t slot, const juce::Rectangle<int>& topLeftBounds) {
  const auto& controller1 = controllers_[slot][0];
  const auto& controller2 = controllers_[slot][1];
  auto& controller3 = controllers_[slot][2];

  if ((controller2->getY() == topLeftBounds.getBottom()) ||
      (controller2->getX() != controller1->getX() &&
       controller2->getY() == controller1->getY())) {
    // Placed at the same position of controller 2 since SR is no effect.
    controller3->setTopLeftPosition(controller2->getPosition());
    return;
  }

  const auto movableTopLeftArea = [&] {
    const auto controller1TopLeftPosition = controller1->getPosition();
    const auto controller2TopLeftPosition = controller2->getPosition();
    const auto controller2MovableTopLeftArea =
        topLeftBounds.withTrimmedLeft(controller1TopLeftPosition.x)
            .withTrimmedTop(controller1TopLeftPosition.y)
            .withWidth(kMaxDrSrWidth);
    const auto scale =
        .5f * (topLeftBounds.getBottom() - controller2TopLeftPosition.getY()) /
        controller2MovableTopLeftArea.getHeight();
    return controller2MovableTopLeftArea
        .withPosition(controller2TopLeftPosition)
        .transformedBy(juce::AffineTransform::scale(
            scale, scale, static_cast<float>(controller2TopLeftPosition.x),
            static_cast<float>(controller2TopLeftPosition.y)));
  }();

  const auto sr = juce::roundToInt(parameters_
                                       .getRawParameterValue(ap::idAsString(
                                           slot, ap::FmOperatorParameter::Sr))
                                       ->load());
  if (sr == ap::SustainRateValue::kMinimum) {
    controller3->setTopLeftPosition(
        movableTopLeftArea.getTopLeft().translated(kMaxDrSrWidth, 0));
    return;
  }

  const auto left =
      juce::jmap<int>(sr, ap::SustainRateValue::kMaximum, kMinSlopedSr,
                      movableTopLeftArea.getX(), movableTopLeftArea.getRight());

  controller3->setTopLeftPosition(left, movableTopLeftArea.getBottom());
}

void EnvelopeGraph::updateTopLeftPositionOfController4(
    std::size_t slot, const juce::Rectangle<int>& topLeftBounds) {
  const auto& controller3 = controllers_[slot][2];
  auto& controller4 = controllers_[slot][3];

  if (controller3->getY() == topLeftBounds.getBottom()) {
    // Placed at the same position of controller 3 since RR is no effect.
    controller4->setTopLeftPosition(controller3->getPosition());
    return;
  }

  const auto rr = juce::roundToInt(parameters_
                                       .getRawParameterValue(ap::idAsString(
                                           slot, ap::FmOperatorParameter::Rr))
                                       ->load());

  if (rr == ap::ReleaseRateValue::kMinimum) {
    controller4->setTopLeftPosition(
        controller3->getPosition().withX(topLeftBounds.getRight()));
    return;
  }

  const auto left =
      juce::jmap<int>(rr, ap::ReleaseRateValue::kMaximum, kMinSlopedRr,
                      controller3->getX(), controller3->getX() + kMaxRrWidth);

  controller4->setTopLeftPosition(left, topLeftBounds.getBottom());
}

void EnvelopeGraph::paint(juce::Graphics& graphics) {
  // Background.
  graphics.fillAll(colour::kBackground);

  // Flip y-axis for convenience.
  const auto flipYAxis =
      juce::AffineTransform::verticalFlip(static_cast<float>(getHeight()));
  graphics.addTransform(flipYAxis);

  const auto sortedSlotIndices = [&] {
    auto itr = std::ranges::find_if(controllers_, [&](auto&& cs) {
      return cs[0] == visibleControllers_[0];
    });
    const auto visibleIndex =
        static_cast<size_t>(std::distance(std::begin(controllers_), itr));
    std::array<std::size_t, kControllerCount_> indices;
    std::iota(std::begin(indices), std::end(indices), 0u);
    std::ranges::remove(indices, visibleIndex).front() = visibleIndex;
    return indices;
  }();

  for (std::size_t slot : sortedSlotIndices) {
    const auto& controllers = controllers_[slot];

    std::vector<juce::Point<float>> points;
    std::ranges::transform(
        controllers, std::back_inserter(points),
        [&flipYAxis](auto&& controller) {
          return controller->getBounds().toFloat().getCentre().transformedBy(
              flipYAxis);
        });

    // Envelope.
    juce::Path envelopePath;
    envelopePath.startNewSubPath({});
    // Draw a line as cubic-bezier, but actual Attack is exponential change.
    envelopePath.cubicTo(points[0].x * .5f, points[0].y * .8f,
                         points[0].x * .7f, points[0].y, points[0].x,
                         points[0].y);
    envelopePath.lineTo(points[1]);
    envelopePath.lineTo(points[2]);
    envelopePath.lineTo(points[3]);

    const float strokeThickness = linkStrokeThicknessList_[slot];

    if (points[3].x == getLocalBounds().toFloat().getRight()) {
      // Close path to fill colour.
      const auto outerPoint = points[3].translated(strokeThickness, 0.f);
      envelopePath.lineTo(outerPoint);
      envelopePath.lineTo(outerPoint.withY(0.f));
    }

    const auto linkColour =
        (controllers == visibleControllers_)
            ? colour::kLink[slot]
            : colour::kLink[slot].withLightness(colour::kLineBackLightness);
    graphics.setColour(linkColour);
    graphics.strokePath(envelopePath, juce::PathStrokeType{strokeThickness});

    if (controllers != visibleControllers_) {
      continue;
    }

    const auto gradientColour = [linkColour,
                                 y = static_cast<float>(points[0].y)] {
      auto gradient = juce::ColourGradient::vertical(colour::kBackground, 0.f,
                                                     linkColour, y);
      gradient.multiplyOpacity(colour::kEnvelopeFillGradientOpacityMultiply);
      return gradient;
    }();
    graphics.setGradientFill(gradientColour);
    envelopePath.closeSubPath();
    graphics.fillPath(envelopePath);
  }
}

void EnvelopeGraph::resized() {
  const auto quarterWidth = getWidth() / 4;

  kMaxArWidth = quarterWidth;
  kMaxDrSrWidth = quarterWidth;
  kMaxRrWidth = quarterWidth;

  updateControllerPosition();
}

void EnvelopeGraph::setFrontEnvelopeOperator(std::size_t slot) {
  if (audio::kSlotCount <= slot) {
    return;
  }

  if (linkStrokeThicknessList_[slot] == stroke_thickness::kFrontLink) {
    // Not need to update state.
    return;
  }

  frontArId_ = ap::idAsString(slot, ap::FmOperatorParameter::Ar);
  frontTlId_ = ap::idAsString(slot, ap::FmOperatorParameter::Tl);
  frontDrId_ = ap::idAsString(slot, ap::FmOperatorParameter::Dr);
  frontSlId_ = ap::idAsString(slot, ap::FmOperatorParameter::Sl);
  frontSrId_ = ap::idAsString(slot, ap::FmOperatorParameter::Sr);
  frontRrId_ = ap::idAsString(slot, ap::FmOperatorParameter::Rr);

  for (std::size_t i = 0u; i < audio::kSlotCount; ++i) {
    const bool isVisible = i == slot;

    for (auto& controller : controllers_[i]) {
      controller->setVisible(isVisible);
    }

    if (isVisible) {
      visibleControllers_ = controllers_[i];
    }

    linkStrokeThicknessList_[i] =
        isVisible ? stroke_thickness::kFrontLink : stroke_thickness::kBackLink;
  }
}
}  // namespace ui
