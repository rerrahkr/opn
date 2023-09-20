// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Rerrah

#include "parameter.h"

#include <unordered_map>

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
        {FmOperatorParameter::OperatorEnabled,
         {"operatorEnabled", "Operator Enabled"}},
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
}  // namespace audio
