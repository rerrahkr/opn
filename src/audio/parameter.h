// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <compare>
#include <variant>

namespace audio {
namespace parameter {
/// Parameters related to plugin behavior.
enum class PluginParameter {
  PitchBendSensitivity,
};

/**
 * @brief Get parameter identifier.
 *
 * @param[in] type Parameter type.
 * @return Parameter identifier.
 */
juce::ParameterID id(PluginParameter type);

/**
 * @brief Get paraemter identifier as \c juce::String.
 *
 * @param[in] type parameter type.
 * @return Parameter identifier text.
 */
juce::String idAsString(PluginParameter type);

/**
 * @brief Get parameter name.
 *
 * @param[in] type Parameter type.
 * @return Parameter name text.
 */
juce::String name(PluginParameter type);

//==============================================================================
/// Parameters related to tone.
enum class FmToneParameter {
  Al,
  Fb,
  LfoEnabled,
  LfoFrequency,
  Pms,
  Ams,
};

/**
 * @brief Get parameter identifier.
 *
 * @param[in] type Parameter type.
 * @return Parameter identifier.
 */
juce::ParameterID id(FmToneParameter type);

/**
 * @brief Get paraemter identifier as \c juce::String.
 *
 * @param[in] type parameter type.
 * @return Parameter identifier text.
 */
juce::String idAsString(FmToneParameter type);

/**
 * @brief Get parameter name.
 *
 * @param[in] type Parameter type.
 * @return Parameter name text.
 */
juce::String name(FmToneParameter type);

//==============================================================================
/// Parameters related to FM opeator.
enum class FmOperatorParameter {
  Ar,
  Dr,
  Sr,
  Rr,
  Sl,
  Tl,
  Ks,
  Ml,
  Amon,
  SsgegEnabled,
  SsgegShape,
};

/**
 * @brief Get parameter identifier.
 *
 * @param[in] slot Slot number. It must be set between 0 and 3.
 * @param[in] type Parameter type.
 * @return Parameter identifier.
 */
juce::ParameterID id(std::size_t slot, FmOperatorParameter type);

/**
 * @brief Get paraemter identifier as \c juce::String.
 *
 * @param[in] slot Slot number. It must be set between 0 and 3.
 * @param[in] type parameter type.
 * @return Parameter identifier text.
 */
juce::String idAsString(std::size_t slot, FmOperatorParameter type);

/**
 * @brief Get parameter name.
 *
 * @param[in] slot Slot number. It must be set between 0 and 3.
 * @param[in] type Parameter type.
 * @return Parameter name text.
 */
juce::String name(std::size_t slot, FmOperatorParameter type);

/// The pair of FM operator's parameter and slot number.
struct FmOperatorParameterWithSlot {
  std::size_t slot;
  FmOperatorParameter parameter;

  /**
   * @brief Constructor.
   * @param[in] slot Slot number.
   * @param[in] parameter Parameter.
   */
  FmOperatorParameterWithSlot(std::size_t slot, FmOperatorParameter parameter)
      : slot(slot), parameter(parameter) {}

  auto operator<=>(const FmOperatorParameterWithSlot&) const = default;
};
}  // namespace parameter
}  // namespace audio

namespace std {
template <>
struct hash<audio::parameter::FmOperatorParameterWithSlot> {
  std::size_t operator()(
      const audio::parameter::FmOperatorParameterWithSlot& parameter) const {
    return std::hash<std::string>()(
        std::to_string(parameter.slot) + "," +
        std::to_string(static_cast<int>(parameter.parameter)));
  }
};
}  // namespace std

namespace audio {
namespace parameter {
/// Shape of SSG-EG.
enum class SsgegShape : std::uint8_t {
  DownwardSaw = 8,
  FadeOut,
  DownwardTriangle,
  FadeOutAndSoundAgain,
  UpwardSaw,
  FadeIn,
  UpwardTriangle,
  FadeInAndSilence
};
}  // namespace parameter

//==============================================================================
/// Parameter helper type which contains any parameter.
using Parameter =
    std::variant<parameter::PluginParameter, parameter::FmToneParameter,
                 parameter::FmOperatorParameterWithSlot>;

class FmAudioSource;

/// Visitor functions for parameter helper type.
class ParameterVisiter {
 public:
  /**
   * @brief Constructor.
   * @param[in] audioSource Audio source.
   * @param[in] apvts Parameter state of plugin.
   */
  ParameterVisiter(FmAudioSource& audioSource,
                   juce::AudioProcessorValueTreeState& apvts);

  void operator()(parameter::PluginParameter parameter);
  void operator()(parameter::FmToneParameter parameter);
  void operator()(const parameter::FmOperatorParameterWithSlot& parameter);

 private:
  FmAudioSource& audioSource_;
  juce::AudioProcessorValueTreeState& apvts_;
};
}  // namespace audio

//==============================================================================
namespace audio {
/**
 * @brief FM parameters.
 */
struct FmParameters {
  std::uint8_t al{7u}, fb{0u};

  /// Parameters related to operator.
  struct Operator {
    std::uint8_t ar{31u}, dr{0u}, sr{0u}, rr{7u}, sl{0u}, tl{0u}, ks{0u},
        ml{0u};
    std::int8_t dt{0};

    /// Data of SSG-EG
    struct Ssgeg {
      parameter::SsgegShape shape{parameter::SsgegShape::DownwardSaw};
      bool isEnabled{};
    };

    Ssgeg ssgeg{};

    bool am{};

    bool isEnabled{true};  ///< Whether this operator is enabled.
  };

  Operator op[4]{};

  /// Parameters related on LFO.
  struct Lfo {
    std::int8_t frequency{}, pms{}, ams{};
    bool isEnabled{};
  };

  Lfo lfo;
};
}  // namespace audio
