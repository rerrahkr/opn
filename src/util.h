// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <type_traits>

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
}  // namespace util
