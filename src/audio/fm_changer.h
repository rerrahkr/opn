// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <set>
#include <utility>
#include <vector>

#include "parameter.h"
#include "register.h"

namespace audio {
/**
 * @brief Functor to change FM feedback parameter.
 */
class FmFeedbackChanger {
 public:
  explicit FmFeedbackChanger(int value)
      : value_(static_cast<decltype(value_)::ValueType>(value)) {}

  std::pair<FmParameters, std::vector<Register>> operator()(
      const FmParameters& parameters, const std::set<std::size_t> ids) const;

 private:
  FmParameters::FeedbackValue value_;
};

/**
 * @brief Functor to change FM algorithm parameter.
 */
class FmAlgorithmChanger {
 public:
  explicit FmAlgorithmChanger(int value)
      : value_(static_cast<decltype(value_)::ValueType>(value)) {}

  std::pair<FmParameters, std::vector<Register>> operator()(
      const FmParameters& parameters, const std::set<std::size_t> ids) const;

 private:
  FmParameters::AlgorithmValue value_;
};
}  // namespace audio
