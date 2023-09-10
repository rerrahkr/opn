// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "parameter.h"

#include <concepts>
#include <unordered_map>
#include <utility>

#include "fm_audio_source.h"

namespace audio {
namespace parameter {
namespace {
const std::unordered_map<PluginParameter,
                         std::pair<juce::ParameterID, juce::String>>
    kPluginIdNameLookUp_{
        {PluginParameter::PitchBendSensitivity,
         {"pitchBendSensitivity", "Pitch Bend Sensitivity"}},
    };
}

juce::ParameterID id(PluginParameter type) {
  return kPluginIdNameLookUp_.at(type).first;
}

juce::String idAsString(PluginParameter type) {
  return kPluginIdNameLookUp_.at(type).first.getParamID();
}

juce::String name(PluginParameter type) {
  return kPluginIdNameLookUp_.at(type).second;
}

//==============================================================================
namespace {
const std::unordered_map<FmToneParameter,
                         std::pair<juce::ParameterID, juce::String>>
    kToneIdNameLookUp_{
        {FmToneParameter::Al, {"al", "Algorithm"}},
        {FmToneParameter::Fb, {"fb", "Feedback"}},
        {FmToneParameter::LfoEnabled, {"lfoEnabled", "LFO Enabled"}},
        {FmToneParameter::LfoFrequency, {"lfoFrequency", "LFO Frequency"}},
        {FmToneParameter::Pms, {"pms", "Phase Modulation Sensitivity"}},
        {FmToneParameter::Ams, {"ams", "Amplitude Modulation Sensitivity"}},
    };
}
juce::ParameterID id(FmToneParameter type) {
  return kToneIdNameLookUp_.at(type).first;
}

juce::String idAsString(FmToneParameter type) {
  return kToneIdNameLookUp_.at(type).first.getParamID();
}

juce::String name(FmToneParameter type) {
  return kToneIdNameLookUp_.at(type).second;
}

//==============================================================================
namespace {
const std::unordered_map<FmOperatorParameter,
                         std::pair<juce::String, juce::String>>
    kOperatorIdNameLookUp_{
        {FmOperatorParameter::Ar, {"ar", "Attack Rate"}},
        {FmOperatorParameter::Dr, {"dr", "Decay Rate"}},
        {FmOperatorParameter::Sr, {"sr", "Sustain Rate"}},
        {FmOperatorParameter::Rr, {"rr", "Release Rate"}},
        {FmOperatorParameter::Sl, {"sl", "Sustain Level"}},
        {FmOperatorParameter::Tl, {"tl", "Total Level"}},
        {FmOperatorParameter::Ks, {"ks", "Key Scale"}},
        {FmOperatorParameter::Ml, {"ml", "Multiple"}},
        {FmOperatorParameter::Dt, {"dt", "Detune"}},
        {FmOperatorParameter::Amon, {"amon", "Amplitude Modulation Enabled"}},
        {FmOperatorParameter::SsgegEnabled, {"ssgegEnabled", "SSG-EG Enabled"}},
        {FmOperatorParameter::SsgegShape, {"ssgegShape", "SSG-EG Shape"}},
    };
}

juce::ParameterID id(std::size_t slot, FmOperatorParameter type) {
  return juce::ParameterID{idAsString(slot, type)};
}

juce::String idAsString(std::size_t slot, FmOperatorParameter type) {
  return kOperatorIdNameLookUp_.at(type).first + juce::String(slot + 1);
}

juce::String name(std::size_t slot, FmOperatorParameter type) {
  return kOperatorIdNameLookUp_.at(type).second + " " + juce::String(slot + 1);
}
}  // namespace parameter

ParameterVisiter::ParameterVisiter(FmAudioSource& audioSource,
                                   juce::AudioProcessorValueTreeState& apvts)
    : audioSource_(audioSource), apvts_(apvts) {}

void ParameterVisiter::operator()(parameter::PluginParameter parameter) {
  switch (parameter) {
    default:
      break;

    case parameter::PluginParameter::PitchBendSensitivity: {
      audioSource_.tryReservePitchBendSensitivityChange(static_cast<int>(
          apvts_
              .getRawParameterValue(parameter::idAsString(
                  parameter::PluginParameter::PitchBendSensitivity))
              ->load()));
      break;
    }
  }
}

namespace {
template <class T>
concept HasValueType = requires() { typename T::ValueType; };

/**
 * @brief Convert raw value to parameter value.
 * @tparam To Target type.
 * @tparam From Original type.
 * @param[in] value Raw value.
 * @return Casted value as @c To.
 */
template <HasValueType To, typename From>
  requires std::convertible_to<From, typename To::ValueType>
auto parameterCast(From&& value) {
  return To(static_cast<To::ValueType>(value));
}

/**
 * @brief Get parameter value from value tree state as a value of given type.
 * @tparam T Type of returned value.
 * @param[in] apvts Audio processor value tree state.
 * @param[in] parameter Type of parameter.
 * @return Value of parameter as specified type.
 */
template <HasValueType T>
auto getParameterValue(const juce::AudioProcessorValueTreeState& apvts,
                       parameter::FmToneParameter parameter) {
  return parameterCast<T>(
      apvts.getRawParameterValue(parameter::idAsString(parameter))->load());
}

/**
 * @brief Get parameter value from value tree state as a value of given type.
 * @tparam T Type of returned value.
 * @param[in] apvts Audio processor value tree state.
 * @param[in] slot Slot number.
 * @param[in] parameter Type of parameter.
 * @return Value of parameter as specified type.
 */
template <HasValueType T>
auto getParameterValue(const juce::AudioProcessorValueTreeState& apvts,
                       std::size_t slot,
                       parameter::FmOperatorParameter parameter) {
  return parameterCast<T>(
      apvts.getRawParameterValue(parameter::idAsString(slot, parameter))
          ->load());
}
}  // namespace

void ParameterVisiter::operator()(parameter::FmToneParameter parameter) {
  switch (parameter) {
    using enum parameter::FmToneParameter;

    case Fb:
      audioSource_.tryReserveFeedbackChange(
          getParameterValue<parameter::FeedbackValue>(apvts_, parameter));
      break;

    case Al:
      audioSource_.tryReserveAlgorithmChange(
          getParameterValue<parameter::AlgorithmValue>(apvts_, parameter));
      break;

    default:
      break;
  }
}

void ParameterVisiter::operator()(
    const parameter::FmOperatorParameterWithSlot& parameter) {
  switch (parameter.parameter) {
    using enum parameter::FmOperatorParameter;

    case Ar:
      audioSource_.tryReserveAttackRateChange(
          parameter.slot, getParameterValue<parameter::AttackRateValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Dr:
      audioSource_.tryReserveDecayRateChange(
          parameter.slot, getParameterValue<parameter::DecayRateValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Sr:
      audioSource_.tryReserveSustainRateChange(
          parameter.slot, getParameterValue<parameter::SustainRateValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Rr:
      audioSource_.tryReserveReleaseRateChange(
          parameter.slot, getParameterValue<parameter::ReleaseRateValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Sl:
      audioSource_.tryReserveSustainLevelChange(
          parameter.slot, getParameterValue<parameter::SustainLevelValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Tl:
      audioSource_.tryReserveTotalLevelChange(
          parameter.slot, getParameterValue<parameter::TotalLevelValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Ks:
      audioSource_.tryReserveKeyScaleChange(
          parameter.slot, getParameterValue<parameter::KeyScaleValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Ml:
      audioSource_.tryReserveMultipleChange(
          parameter.slot, getParameterValue<parameter::MultipleValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    case Dt:
      audioSource_.tryReserveDetuneChange(
          parameter.slot, getParameterValue<parameter::DetuneValue>(
                              apvts_, parameter.slot, parameter.parameter));
      break;

    default:
      break;
  }
}
}  // namespace audio
