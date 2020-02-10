cmake_minimum_required (VERSION 3.8)
project (libdsp)

# Used for autocompletion in vim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow static library to be included in another lib (used for python wrapping)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(dependencies)

add_library(dsp STATIC "")

add_subdirectory(src)

target_include_directories(dsp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_link_libraries(dsp
    PUBLIC common armadillo)

target_compile_options(dsp PUBLIC -Werror -Wall -Wextra)

# Tests
include(CTest)
if(DSP_RUNTESTS)
    enable_testing()
    add_subdirectory(test)
endif()
