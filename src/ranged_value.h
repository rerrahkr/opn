// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <algorithm>
#include <compare>
#include <concepts>

/// Numeric type concept.
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

/**
 * @brief Ranged value class.
 *
 * @tparam T Type of value.
 */
template <Numeric T, T Minimum, T Maximum>
  requires(Minimum <= Maximum)
class RangedValue {
 public:
  /// Type of value.
  using ValueType = T;

  /**
   * @brief Constructor that the initial value is set to \c Minimum.
   */
  constexpr RangedValue() noexcept : value_(Minimum) {}

  /**
   * @brief Constructor with initial value.
   *
   * @param[in] value Initial value.
   */
  constexpr RangedValue(T value) noexcept { setAndClampValue(value); }

  /**
   * @brief Get value.
   *
   * @return Value.
   */
  constexpr T rawValue() const noexcept { return value_; }

  /**
   * @brief Set given value if it is valid.
   *
   * @param value A new value.
   * @return \c true if the value is changed. If a new value is out of range,
   * returns \c false.
   */
  constexpr bool trySetValue(T value) noexcept {
    if (value < Minimum || Maximum < value) {
      return false;
    }

    value_ = value;
    return true;
  }

  /**
   * @brief Set given value and clamp it.
   *
   * @param value A new value.
   */
  constexpr void setAndClampValue(T value) noexcept {
    value_ = std::clamp(value, Minimum, Maximum);
  }

  /// Minimum value.
  static constexpr T kMinimum{Minimum};

  /// Maximum value.
  static constexpr T kMaximum{Maximum};

  auto operator<=>(const RangedValue&) const = default;

 private:
  T value_;  ///< A value.
};

template <Numeric T, T... Args>
constexpr bool operator==(const RangedValue<T, Args...>& left, T right) {
  return left.rawValue() == right;
}

template <Numeric T, T... Args>
constexpr bool operator==(T left, const RangedValue<T, Args...>& right) {
  return right == left;
}

template <Numeric T, T... Args>
constexpr bool operator!=(const RangedValue<T, Args...>& left, T right) {
  return !(left == right);
}

template <Numeric T, T... Args>
constexpr bool operator!=(T left, const RangedValue<T, Args...>& right) {
  return right != left;
}

template <Numeric T, T... Args>
constexpr bool operator<(const RangedValue<T, Args...>& left, T right) {
  return left.rawValue() < right;
}

template <Numeric T, T... Args>
constexpr bool operator<(T left, const RangedValue<T, Args...> right) {
  return left < right.rawValue();
}

template <Numeric T, T... Args>
constexpr bool operator>(const RangedValue<T, Args...>& left, T right) {
  return right < left;
}

template <Numeric T, T... Args>
constexpr bool operator>(T left, const RangedValue<T, Args...> right) {
  return right < left;
}

template <Numeric T, T... Args>
constexpr bool operator<=(const RangedValue<T, Args...>& left, T right) {
  return !(right < left);
}

template <Numeric T, T... Args>
constexpr bool operator<=(T left, const RangedValue<T, Args...> right) {
  return !(right < left);
}

template <Numeric T, T... Args>
constexpr bool operator>=(const RangedValue<T, Args...>& left, T right) {
  return !(left < right);
}

template <Numeric T, T... Args>
constexpr bool operator>=(T left, const RangedValue<T, Args...> right) {
  return !(left < right);
}
