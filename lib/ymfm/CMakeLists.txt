cmake_minimum_required(VERSION 3.13.0)

project(ymfm CXX)

add_library(ymfm)

set(YMFM_SRC ymfm/src)
target_sources(ymfm
    PRIVATE
        ${YMFM_SRC}/ymfm_adpcm.cpp
        ${YMFM_SRC}/ymfm_misc.cpp
        ${YMFM_SRC}/ymfm_opn.cpp
        ${YMFM_SRC}/ymfm_ssg.cpp)

target_include_directories(ymfm PUBLIC ${YMFM_SRC})

target_compile_features(ymfm PRIVATE cxx_std_20)

include(${CMAKE_SOURCE_DIR}/cmake/config.cmake)
set(COMPILE_FLAGS)
get_compile_flags(COMPILE_FLAGS)
target_compile_options(ymfm PRIVATE ${COMPILE_FLAGS})
