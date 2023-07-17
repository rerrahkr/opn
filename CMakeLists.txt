cmake_minimum_required(VERSION 3.13.0)

set(PROJECT_TARGET jucetemplate)

project(${PROJECT_TARGET} VERSION 0.1.0 LANGUAGES CXX)

set(CPACK_PROJECT_NAME ${PROJECT_TARGET})
set(CPACK_PROJECT_VERSION ${PROJECT_TARGET})
include(CPack)

set(COMPILER_FLAGS)
if(MSVC)
    list(APPEND COMPILER_FLAGS /O2 /Wall /WX /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
    list(APPEND COMPILER_FLAGS -O2 -Wall -Wextra -Werror -pedantic -pedantic-errors)
endif()

add_subdirectory(lib/juce)

juce_add_plugin(${PROJECT_TARGET}
    COMPANY_NAME "Rerrah"
    COMPANY_COPYRIGHT "Rerrah"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Rera
    PLUGIN_CODE Temp
    FORMATS Standalone #VST3
    # VST3_CATEGORIES "Fx"
    PRODUCT_NAME "JUCETemplate")

juce_generate_juce_header(${PROJECT_TARGET})

add_subdirectory(src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_FLAGS})

# target_include_directories(${PROJECT_TARGET} PRIVATE include)

target_link_libraries(${PROJECT_TARGET}
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(${PROJECT_TARGET}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

#include(CTest)
#enable_testing()
#add_subdirectory(test)
