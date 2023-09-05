// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <compare>
#include <variant>

#include "../ranged_value.h"

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
  using AlgorithmValue = RangedValue<std::uint8_t, 0, 7>;
  using FeedbackValue = RangedValue<std::uint8_t, 0, 7>;

  AlgorithmValue al{7u};
  FeedbackValue fb{0u};

  /// Parameters related to operator.
  struct Operator {
    RangedValue<std::uint8_t, 0, 31> ar{31u}, dr{0u}, sr{0u};
    RangedValue<std::uint8_t, 0, 15> rr{7u}, sl{0u};
    RangedValue<std::uint8_t, 0, 127> tl{0u};
    RangedValue<std::uint8_t, 0, 3> ks{0u};
    RangedValue<std::uint8_t, 0, 15> ml{0u};
    RangedValue<std::int8_t, -3, 3> dt{0};

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
    RangedValue<std::uint8_t, 0, 7> frequency{};
    RangedValue<std::uint8_t, 0, 7> pms{};
    RangedValue<std::uint8_t, 0, 3> ams{};
    bool isEnabled{};
  };

  Lfo lfo;

  /// Default parameters.
  static const FmParameters defaultParameters;
};

inline const FmParameters FmParameters::defaultParameters{};
}  // namespace audio
