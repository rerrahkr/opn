// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <utility>

namespace ui {
class NestableGrid;

/**
 * @brief Grid item for @c NestableGrid.
 */
class NestableGridItem : public juce::GridItem {
 public:
  NestableGridItem() noexcept = default;

  /**
   * @brief It is the same as a constructor of juce::GridItem.
   * @param[in] item Item.
   */
  NestableGridItem(juce::Component& item) noexcept;

  /**
   * @brief It is the same as a constructor of juce::GridItem.
   * @param[in] item Item.
   */
  NestableGridItem(juce::Component* item) noexcept;

  /**
   * @brief Constructor for inner rectangle.
   * @param[in] item Inner rectangle.
   */
  NestableGridItem(juce::Rectangle<int>& item) noexcept;

  /**
   * @brief Constructor for inner rectangle.
   * @param[in] item Inner rectangle.
   */
  NestableGridItem(juce::Rectangle<int>* item) noexcept;

  /**
   * @brief Constructor for inner grid.
   * @param[in] item Inner grid.
   */
  NestableGridItem(NestableGrid& item) noexcept;

  /**
   * @brief Constructor for inner grid.
   * @param[in] item Inner grid.
   */
  NestableGridItem(NestableGrid* item) noexcept;

  /**
   * @brief Get inner rectangle.
   * @return A pointer of inner rectangle, or @c nullptr if this item does not
   * have a inner rectangle.
   */
  juce::Rectangle<int>* innerRectangle() const noexcept;

  /**
   * @brief Get inner grid.
   * @return A pointer of inner grid, or @c nullptr if this item does not have a
   * inner grid.
   */
  NestableGrid* innerGrid() const noexcept;

 private:
  NestableGrid* grid_;
  juce::Rectangle<int>* rectangle_;
};

/**
 * @brief Grid which is enabled to have inner grids.
 */
class NestableGrid {
 public:
  /**
   * @brief Set gap of rows and columns.
   * @param[in] sizeInPixels Gap size.
   */
  void setGap(juce::Grid::Px sizeInPixels);

  /**
   * @brief Define rows like @c grid-template-rows in CSS.
   * @param[in] rows Definition.
   */
  void setTemplateRows(const juce::Array<juce::Grid::TrackInfo>& rows);

  /**
   * @brief Define columns like @c grid-template-columns in CSS.
   * @param[in] columns Definition.
   */
  void setTemplateColumns(const juce::Array<juce::Grid::TrackInfo>& columns);

  /**
   * @brief Define areas like @c grid-template-areas in CSS.
   * @param[in] areas Definition.
   */
  void setTemplateAreas(const juce::StringArray& areas);

  /**
   * @brief Set items to grid.
   * @param[in] items Items.
   */
  void setItems(const juce::Array<NestableGridItem>& items);

  /**
   * @brief Arrange items.
   * @param[in] targetArea Area to fit items.
   */
  void performLayout(juce::Rectangle<int> targetArea);

 private:
  juce::Grid grid_;

  /// Array of inner grids.
  juce::Array<std::pair<int, NestableGrid*>> innerGrids_;

  /// Array of inner rectangles.
  juce::Array<std::pair<int, juce::Rectangle<int>*>> innerRectangles_;
};
}  // namespace ui
