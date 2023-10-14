// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <type_traits>
#include <utility>

namespace util {
/**
 * @brief Cast enum to value as underlying type.
 * @param[in] value Value as enum.
 * @return Value as underlying type.
 * @remark Equals to \c std::to_underlying() in C++23.
 */
template <typename T>
constexpr auto to_underlying(T value) noexcept {
  return static_cast<std::underlying_type_t<T>>(value);
}

/**
 * @brief Quantize value.
 * @param[in] sourceValue Original value in source range.
 * @param[in] sourceRangeMin Minimum value in source range.
 * @param[in] sourceRangeMax Maximum value in source range.
 * @param[in] quantizedRangeMin Minimum value in quantized range.
 * @param[in] quantizedRangeMax Maximum value in quantized range.
 * @return A pair of value in quantized range and quantized value in source
 * range.
 */
template <typename T>
auto quantize(T sourceValue, T sourceRangeMin, T sourceRangeMax,
              T quantizedRangeMin, T quantizedRangeMax) {
  const auto quantizedValue = juce::roundToInt(
      juce::jmap<double>(sourceValue, sourceRangeMin, sourceRangeMax,
                         quantizedRangeMin, quantizedRangeMax));
  const auto quantizedSource =
      juce::jmap(quantizedValue, quantizedRangeMin, quantizedRangeMax,
                 sourceRangeMin, sourceRangeMax);
  return std::make_pair(quantizedValue, quantizedSource);
}
}  // namespace util
