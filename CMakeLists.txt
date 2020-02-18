cmake_minimum_required (VERSION 3.8)
project (libdsp)

# Used for autocompletion in vim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow static library to be included in another lib (used for python wrapping)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(dsp INTERFACE)

add_subdirectory(src)
add_subdirectory(dependencies)

target_include_directories(dsp
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

target_link_libraries(dsp INTERFACE common sigpack)

target_compile_options(dsp INTERFACE -Werror -Wall -Wextra)

# Tests
include(CTest)
if(DSP_RUNTESTS)
    enable_testing()
    add_subdirectory(test)
endif()
