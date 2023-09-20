// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#pragma once

#include <JuceHeader.h>

#include <compare>
#include <concepts>
#include <utility>
#include <variant>

#include "../../ranged_value.h"
#include "../../toggled_value.h"

namespace audio {
/// Slot count.
static constexpr std::size_t kSlotCount{4u};

namespace parameter {
/**
 * @brief Concept of parameter value.
 */
template <class T>
concept ParameterValue = requires(T parameter) {
  typename T::ValueType;
  parameter.rawValue();
};

/**
 * @brief Pair of slot number and parameter value.
 */
template <ParameterValue T>
struct SlotAndValue {
  RangedValue<std::size_t, 0, kSlotCount - 1u> slot;
  T value;

  /**
   * @brief Constructor.
   * @param[in] slot Slot number.
   * @param[in] value Parameter value.
   */
  SlotAndValue(const decltype(slot)& slot, T&& value)
      : slot(slot), value(std::forward<T>(value)) {}
};

// Parameter value types.
struct PitchBendSensitivityValue : public RangedValue<int, 1, 24> {};

struct OperatorEnabledValue : public ToggledValue {};
struct AlgorithmValue : public RangedValue<std::uint8_t, 0, 7> {};
struct FeedbackValue : public RangedValue<std::uint8_t, 0, 7> {};
struct AttackRateValue : public RangedValue<std::uint8_t, 0, 31> {};
struct DecayRateValue : public RangedValue<std::uint8_t, 0, 31> {};
struct SustainRateValue : public RangedValue<std::uint8_t, 0, 31> {};
struct ReleaseRateValue : public RangedValue<std::uint8_t, 0, 15> {};
struct SustainLevelValue : public RangedValue<std::uint8_t, 0, 15> {};
struct TotalLevelValue : public RangedValue<std::uint8_t, 0, 127> {};
struct KeyScaleValue : public RangedValue<std::uint8_t, 0, 3> {};
struct MultipleValue : public RangedValue<std::uint8_t, 0, 15> {};
struct DetuneValue : public RangedValue<std::int8_t, -3, 3> {};

struct LfoFrequency : public RangedValue<std::uint8_t, 0, 7> {};
struct LfoPmsValue : public RangedValue<std::uint8_t, 0, 7> {};
struct LfoAmsValue : public RangedValue<std::uint8_t, 0, 3> {};

/**
 * @brief Convert raw value to parameter value.
 * @tparam To Target type.
 * @tparam From Original type.
 * @param[in] value Raw value.
 * @return Casted value as @c To.
 */
template <ParameterValue To, typename From>
  requires std::convertible_to<From, typename To::ValueType>
auto parameterCast(From&& value) {
  return To(static_cast<To::ValueType>(value));
}

/// Variant class of parameter values.
using ParameterVariant =
    std::variant<PitchBendSensitivityValue,

                 AlgorithmValue, FeedbackValue,

                 SlotAndValue<OperatorEnabledValue>,
                 SlotAndValue<AttackRateValue>, SlotAndValue<DecayRateValue>,
                 SlotAndValue<SustainRateValue>, SlotAndValue<ReleaseRateValue>,
                 SlotAndValue<SustainLevelValue>, SlotAndValue<TotalLevelValue>,
                 SlotAndValue<KeyScaleValue>, SlotAndValue<MultipleValue>,
                 SlotAndValue<DetuneValue>/* ,

                 SlotAndValue<LfoFrequency>, SlotAndValue<LfoPmsValue>,
                 SlotAndValue<LfoAmsValue> */>;

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
 * @brief Get paraemter identifier as @c juce::String.
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
 * @brief Get paraemter identifier as @c juce::String.
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
  OperatorEnabled,
  Ar,
  Dr,
  Sr,
  Rr,
  Sl,
  Tl,
  Ks,
  Ml,
  Dt,
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
 * @brief Get paraemter identifier as @c juce::String.
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
/**
 * @brief FM parameters.
 */
struct FmParameters {
  parameter::AlgorithmValue al{7u};
  parameter::FeedbackValue fb{0u};

  /// Parameters related to operator.
  struct Operator {
    parameter::OperatorEnabledValue isEnabled{
        true};  ///< Whether this operator is enabled.
    parameter::AttackRateValue ar{31u};
    parameter::DecayRateValue dr{0u};
    parameter::SustainRateValue sr{0u};
    parameter::ReleaseRateValue rr{7u};
    parameter::SustainLevelValue sl{0u};
    parameter::TotalLevelValue tl{0u};
    parameter::KeyScaleValue ks{0u};
    parameter::MultipleValue ml{0u};
    parameter::DetuneValue dt{0};

    /// Data of SSG-EG
    struct Ssgeg {
      parameter::SsgegShape shape{parameter::SsgegShape::DownwardSaw};
      bool isEnabled{};
    };

    Ssgeg ssgeg{};

    bool am{};
  };

  Operator slot[kSlotCount]{};

  /// Parameters related on LFO.
  struct Lfo {
    parameter::LfoFrequency frequency{};
    parameter::LfoPmsValue pms{};
    parameter::LfoAmsValue ams{};
    bool isEnabled{};
  };

  Lfo lfo{};
};

/// Default FM parameter values.
inline constexpr FmParameters defaultFmParameters{};
}  // namespace audio
