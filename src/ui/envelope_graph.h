// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <array>
#include <memory>
#include <optional>

#include "../audio/parameter/parameter.h"

namespace ui {
/**
 * @brief Controller component to use editing FM envelope.
 */
class ControlPoint : public juce::Component {
 public:
  /**
   * @brief Constructor.
   * @param[in] fillColour Colour of controller.
   * @param[in] borderColour Colour of outline.
   */
  ControlPoint(const juce::Colour& fillColour,
               const juce::Colour& borderColour);

  bool hitTest(int x, int y) override;
  void paint(juce::Graphics& graphics) override;
  void resized() override;

 private:
  /// Shape of controller.
  juce::Path hitTestShape_, drawingShape_;

  juce::Colour fillColour_, borderColour_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPoint)
};

//==============================================================================

/**
 * @brief Graph of FM envelope.
 *
 * @details graph is represented by four controller line segments.
 *
 * |       o C1         :<--Note-off line
 * |      / \           :
 * |     /   L2         :
 * |    /     \         :
 * |   L1      o---L3   :
 * |  /        C2   \---o C3
 * | /                  :\
 * |/                   : \<--L4
 * +--------------------+--o----
 *                         C4
 * [C1: Controller 1]
 * It represents values of Attack Rate and Total Level. When a user changes it
 * with sliders or mouse dragging, it is moved to the position calculated from
 * Attack Rate and Total Level.
 *
 * When Attack Rate is edited, it moves horizontally and subsequent controllers
 * also move by the same amount. When Total Level is edited, it moves vertically
 * and subsequent controllers are scaled vertically.
 *
 * When Attack Rate is 0 (the minimum value) or Total Level is 127 (the maximum
 * value), it is placed at the bottom-left position of draggable area regardless
 * of other value.
 *
 * [C2: Controller 2]
 * It represents values of Decay Rate and Sustain Level. When a user changes it
 * with sliders or mouse dragging, it is moved to the position calculated from
 * Decay Rate and Sustain Level. It cannot be moved to the right or upper
 * position than C1.
 *
 * When Decay Rate is edited, it moves horizontally and subsequent controllers
 * also move by the same amount. When Sustain Level is edited, it moves on the
 * extension line of L2.
 *
 * When Sustain Level is 0 (the minimum value), it is placed at the special
 * position:
 * - Top-left position of draggable area when Decay Rate is 31 (the maximum
 * value) where C1 is placed.
 * - Top-right position of draggable area when Decay Rate is not 31.
 *
 * [C3: Controller 3]
 * It represents value of Sustain Rate. When a user changes it with a slider or
 * mouse dragging, it is moved to the position calculated from Sustain Rate. It
 * cannot be moved to the right or upper position than C2, or the lower half of
 * C2 height.
 *
 * When Sustain Rate is edited, it moves on the extension line of L3.
 *
 * When Sustain Rate is 0 (the minimum value), it is placed at the top-eight
 * position of draggable area.
 *
 * [C4: Controller 4]
 * It represents value of Release Rate. When a user changes it with a slider or
 * mouse dragging, it is moved to the position calculated from Release Rate. It
 * can only be moved on the bottom of the graph and to the right position tha
 * C3.
 *
 * When Release Rate is edited, it moves horizontally.
 *
 * When Release Rate is 0 (the minimum value), it is placed at the right end of
 * the graph at the same hight as C3.
 */
class EnvelopeGraph : public juce::Component {
 public:
  /**
   * @brief Constructor.
   * @param[in] parameters Parameters of plugin.
   */
  EnvelopeGraph(juce::AudioProcessorValueTreeState& parameters);

  // Mouse dragging operations.
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;

  /**
   * @brief Update position of controllers following parameter values of plugin.
   */
  void updateControllerPosition();

  /**
   * @brief Update state.
   * @param[in] state State.
   */
  template <class State>
  void render(const State& state) {
    setFrontEnvelopeOperator(state.envelopeGraphFrontSlot);
    repaint();
  }

  void paint(juce::Graphics& graphics) override;
  void resized() override;

 private:
  static constexpr std::size_t kControllerCount_{4u};

  /// Parameters of plugin.
  juce::AudioProcessorValueTreeState& parameters_;

  /// Draggable control points.
  std::unique_ptr<ControlPoint> controllers_[audio::kSlotCount]
                                            [kControllerCount_];

  /// Current visible controllers.
  std::unique_ptr<ControlPoint>* visibleControllers_{controllers_[0]};

  /// The position where a user starts dragging a controller.
  std::optional<juce::Point<int>> controllerTopLeftPositionAtDragStart_{};

  /// Maximum x-width related rate parameters.
  int kMaxArWidth{}, kMaxDrSrWidth{}, kMaxRrWidth{};

  /// List of stroke thickness for envelope line.
  float linkStrokeThicknessList_[audio::kSlotCount]{};

  /// Parameter ID names.
  juce::String frontArId_, frontTlId_, frontDrId_, frontSlId_, frontSrId_,
      frontRrId_;

  /**
   * @brief Calculate and update AR and DR values from controller 1 position.
   * @param[in] draggingTopLeftPosition Current dragging position of
   * controller 1.
   */
  void onController1Dragged(const juce::Point<int>& draggingTopLeftPosition);

  /**
   * @brief Calculate and update DR and SL values from controller 2 position.
   * @param[in] draggingTopLeftPosition Current dragging position of
   * controller 2.
   */
  void onController2Dragged(const juce::Point<int>& draggingTopLeftPosition);

  /**
   * @brief Calculate and update SR values from controller 3 position.
   * @param[in] draggingTopLeftPosition Current dragging position of
   * controller 3.
   */
  void onController3Dragged(const juce::Point<int>& draggingTopLeftPosition);

  /**
   * @brief Calculate and update RR values from controller 4 position.
   * @param[in] draggingTopLeftPosition Current dragging position of
   * controller 4.
   */
  void onController4Dragged(const juce::Point<int>& draggingTopLeftPosition);

  /**
   * @brief Update position of controller 1 following values of AR and TL.
   * @param[in] slot Operator number.
   * @param[in] topLeftBounds Rectangle of shifted local bounds.
   */
  void updateTopLeftPositionOfController1(
      std::size_t slot, const juce::Rectangle<int>& topLeftBounds);

  /**
   * @brief Update position of controller 2 following values of DR and SL.
   * @param[in] slot Operator number.
   * @param[in] topLeftBounds Rectangle of shifted local bounds.
   */
  void updateTopLeftPositionOfController2(
      std::size_t slot, const juce::Rectangle<int>& topLeftBounds);

  /**
   * @brief Update position of controller 3 following values of SR.
   * @param[in] slot Operator number.
   * @param[in] topLeftBounds Rectangle of shifted local bounds.
   */
  void updateTopLeftPositionOfController3(
      std::size_t slot, const juce::Rectangle<int>& topLeftBounds);

  /**
   * @brief Update position of controller 4 following values of RR.
   * @param[in] slot Operator number.
   * @param[in] topLeftBounds Rectangle of shifted local bounds.
   */
  void updateTopLeftPositionOfController4(
      std::size_t slot, const juce::Rectangle<int>& topLeftBounds);

  /**
   * @brief Set a operator number to draw its envelope on the front.
   * @param[in] slot Operator number.
   */
  void setFrontEnvelopeOperator(std::size_t slot);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeGraph)
};
}  // namespace ui
