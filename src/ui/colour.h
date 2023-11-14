// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include "../audio/parameter/parameter.h"

namespace ui::colour {
namespace graph {
inline constexpr float kSlotStrokeAlpha{.75f}, kSlotFillAlpha{.1f};
inline constexpr float kBackLightness{.30f};
inline constexpr float kEnvelopeFillGradientOpacityMultiply{.2f};

inline const juce::Colour kSlot[audio::kSlotCount]{
    juce::Colour::fromHSL(.42f, .70f, .54f, 1.f),
    juce::Colour::fromHSL(.83f, .70f, .54f, 1.f),
    juce::Colour::fromHSL(.60f, .70f, .54f, 1.f),
    juce::Colour::fromHSL(.13f, .70f, .54f, 1.f)};

inline const auto kBackground = juce::Colour::fromHSL(0.f, 0.f, .12f, 1.f);

namespace algorithm {
inline const juce::Colour kSvgFillSlot[audio::kSlotCount]{
    juce::Colour{0xffffcccc}, juce::Colour{0xffffe6cc},
    juce::Colour{0xffffffcc}, juce::Colour{0xffe6ffcc}};
inline const juce::Colour kSvgStrokeSlot[audio::kSlotCount]{
    juce::Colour{0xffff0000}, juce::Colour{0xffff8000},
    juce::Colour{0xffffff00}, juce::Colour{0xff80ff00}};
inline const juce::Colour kSvgStrokeOut{0xff000000}, kSvgFillOut{0xffe6e6e6};
inline const auto kOut = juce::Colour::fromHSL(0.f, 0.f, 0.7f, 1.f);
}  // namespace algorithm
}  // namespace graph

namespace tab {
inline const juce::Colour kSlot[audio::kSlotCount]{
    juce::Colour::fromHSL(.42f, .30f, .30f, 1.f),
    juce::Colour::fromHSL(.83f, .30f, .30f, 1.f),
    juce::Colour::fromHSL(.60f, .30f, .30f, 1.f),
    juce::Colour::fromHSL(.13f, .30f, .30f, 1.f)};
}
}  // namespace ui::colour
