cmake_minimum_required(VERSION 3.13.0)

set(PROJECT_TARGET opn)

project(${PROJECT_TARGET} VERSION 0.1.0 LANGUAGES CXX)

set(CPACK_PROJECT_NAME ${PROJECT_TARGET})
set(CPACK_PROJECT_VERSION ${PROJECT_TARGET})
include(CPack)

add_subdirectory(lib/juce)

juce_add_plugin(${PROJECT_TARGET}
    PRODUCT_NAME OPN
    COMPANY_COPYRIGHT "(C) 2023 Rerrah"
    COMPANY_NAME Rerrah
    FORMATS Standalone VST3
    # PLUGIN_NAME OPN
    PLUGIN_MANUFACTURER_CODE Rera
    PLUGIN_CODE Opnp
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Instrument Synth)

juce_generate_juce_header(${PROJECT_TARGET})

add_subdirectory(src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

include(${CMAKE_SOURCE_DIR}/cmake/config.cmake)
set(COMPILE_FLAGS)
get_compile_flags(COMPILE_FLAGS)
target_compile_options(${PROJECT_TARGET} PRIVATE ${COMPILE_FLAGS})

# target_include_directories(${PROJECT_TARGET} PRIVATE include)

add_subdirectory(res/img)
add_subdirectory(lib/ymfm)

target_link_libraries(${PROJECT_TARGET}
    PRIVATE
        juce::juce_audio_utils
        res_img
        ymfm
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
