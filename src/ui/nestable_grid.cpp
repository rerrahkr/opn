// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "nestable_grid.h"

namespace ui {
NestableGridItem::NestableGridItem(juce::Component& item) noexcept
    : juce::GridItem(item), grid_(nullptr), rectangle_(nullptr) {}

NestableGridItem::NestableGridItem(juce::Component* item) noexcept
    : juce::GridItem(item), grid_(nullptr), rectangle_(nullptr) {}

NestableGridItem::NestableGridItem(juce::Rectangle<int>& item) noexcept
    : NestableGridItem(&item) {}

NestableGridItem::NestableGridItem(juce::Rectangle<int>* item) noexcept
    : grid_(nullptr), rectangle_(item) {}

NestableGridItem::NestableGridItem(NestableGrid& item) noexcept
    : NestableGridItem(&item) {}

NestableGridItem::NestableGridItem(NestableGrid* item) noexcept
    : grid_(item), rectangle_(nullptr) {}

juce::Rectangle<int>* NestableGridItem::innerRectangle() const noexcept {
  return rectangle_;
}

NestableGrid* NestableGridItem::innerGrid() const noexcept { return grid_; }

void NestableGrid::setGap(juce::Grid::Px sizeInPixels) {
  grid_.setGap(sizeInPixels);
}

void NestableGrid::setTemplateRows(
    const juce::Array<juce::Grid::TrackInfo>& rows) {
  grid_.templateRows = rows;
}

void NestableGrid::setTemplateColumns(
    const juce::Array<juce::Grid::TrackInfo>& columns) {
  grid_.templateColumns = columns;
}

void NestableGrid::setTemplateAreas(const juce::StringArray& areas) {
  grid_.templateAreas = areas;
}

void NestableGrid::setItems(const juce::Array<NestableGridItem>& items) {
  for (int i = 0; i < items.size(); ++i) {
    const auto& item = items[i];
    grid_.items.add(item);

    if (auto* innerGrid = item.innerGrid()) {
      innerGrids_.add(std::make_pair(i, innerGrid));
    } else if (auto* innerRectangle = item.innerRectangle()) {
      innerRectangles_.add(std::make_pair(i, innerRectangle));
    }
  }
}

void NestableGrid::performLayout(juce::Rectangle<int> targetArea) {
  grid_.performLayout(targetArea);

  for (auto [i, innerGrid] : innerGrids_) {
    innerGrid->performLayout(grid_.items[i].currentBounds.toNearestInt());
  }

  for (auto [i, innerRectangle] : innerRectangles_) {
    *innerRectangle = grid_.items[i].currentBounds.toNearestInt();
  }
}
}  // namespace ui
