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
}  // namespace graph

namespace tab {
inline const juce::Colour kSlot[audio::kSlotCount]{
    juce::Colour::fromHSL(.42f, .30f, .30f, 1.f),
    juce::Colour::fromHSL(.83f, .30f, .30f, 1.f),
    juce::Colour::fromHSL(.60f, .30f, .30f, 1.f),
    juce::Colour::fromHSL(.13f, .30f, .30f, 1.f)};
}
}  // namespace ui::colour
