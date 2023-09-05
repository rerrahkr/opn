// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <cstdint>

namespace audio {
/**
 * @brief Value pair of address and data of register.
 */
struct Register {
  /// State of pin A1 which controls data bus in a chip.
  bool pinA1{};

  /// Address where data should be written in register.
  std::uint8_t address{};

  /// Data to write.
  std::uint8_t data{};

  /**
   * @brief Constructor.
   * @param[in] pinA1 State of pin A1.
   * @param[in] address Address where data should be written in register.
   * @param[in] data Data to write.
   */
  Register(bool pinA1, std::uint8_t address, std::uint8_t data)
      : pinA1(pinA1), address(address), data(data){};

  /**
   * @brief Constructor supporting 16-bit expression for address.
   * @param[in] address Bit 0-7 is address. Bit 8 is a flag which represents a
   * state of pin A1.
   * @param[in] data Data to write.
   */
  Register(std::uint16_t address, std::uint8_t data)
      : pinA1(address & 0x0100),
        address(static_cast<std::uint8_t>(address & 0x00ff)),
        data(data) {}
};
}  // namespace audio
