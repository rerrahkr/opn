// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "fm_changer.h"

namespace audio {
namespace {
constexpr std::size_t kMaxChannelCount{6u};

constexpr std::uint16_t kAddressOffsetTableForToneSet[kMaxChannelCount]{
    0x0u, 0x1u, 0x2u, 0x100u, 0x101u, 0x102u};
}  // namespace

std::pair<FmParameters, std::vector<Register>> FmFeedbackChanger::operator()(
    const FmParameters& parameters, const std::set<std::size_t> ids) const {
  FmParameters newParameters = parameters;
  std::vector<Register> changes;

  if (parameters.fb != value_) {
    newParameters.fb = value_;

    for (const std::size_t id : ids) {
      if (kMaxChannelCount <= id) {
        // TODO: Fix polyphonic control
        continue;
      }

      changes.emplace_back(0xb0u | kAddressOffsetTableForToneSet[id],
                           (value_.value() << 3) | parameters.al.value());
    }
  }

  return std::make_pair(std::move(newParameters), std::move(changes));
}

std::pair<FmParameters, std::vector<Register>> FmAlgorithmChanger::operator()(
    const FmParameters& parameters, const std::set<std::size_t> ids) const {
  FmParameters newParameters = parameters;
  std::vector<Register> changes;

  if (parameters.al != value_) {
    newParameters.al = value_;

    for (const std::size_t id : ids) {
      if (kMaxChannelCount <= id) {
        // TODO: Fix polyphonic control
        continue;
      }

      changes.emplace_back(0xb0u | kAddressOffsetTableForToneSet[id],
                           (parameters.fb.value() << 3) | value_.value());
    }
  }

  return std::make_pair(std::move(newParameters), std::move(changes));
}
}  // namespace audio
