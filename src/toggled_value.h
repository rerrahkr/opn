// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <compare>

/**
 * @brief Boolean value class.
 */
class ToggledValue {
 public:
  /// Type of value.
  using ValueType = bool;

  /**
   * @brief Constructor that the initial value is set to @c false.
   */
  constexpr ToggledValue() noexcept : value_{} {}

  /**
   * @brief Constructor with initial value.
   *
   * @param[in] value Initial value.
   */
  constexpr ToggledValue(bool value) noexcept : value_(value) {}

  /**
   * @brief Get value.
   *
   * @return Value.
   */
  constexpr bool rawValue() const noexcept { return value_; }

  constexpr explicit operator bool() const noexcept { return value_; }
  constexpr bool operator!() const noexcept { return !value_; }

  auto operator<=>(const ToggledValue&) const = default;

 private:
  bool value_;  ///< A value.
};
