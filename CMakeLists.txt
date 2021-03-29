cmake_minimum_required (VERSION 3.15)
project (libdsp VERSION 0.1.0 LANGUAGES CXX)

################################################################################
# options
################################################################################
# Used for autocompletion in vim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Allow static library to be included in another lib (used for python wrapping)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(DSP_PROFILE "Enable filters profiling" OFF)
option(DSP_RUNTESTS "Built & run tests" OFF)

################################################################################
# dependencies
################################################################################
# common
if (TARGET common)
else()
    add_subdirectory(dependencies/libcommon)
endif()

# cnpy
add_subdirectory(dependencies/cnpy)
target_include_directories(cnpy-static PUBLIC dependencies/cnpy)

# sigpack
# Get version number
file(READ dependencies/sigpack/sigpack.h sigpack_header)
string(REGEX MATCH "SP_VERSION_MAJOR ([0-9]*)" _ ${sigpack_header})
set(SP_VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "SP_VERSION_MINOR ([0-9]*)" _ ${sigpack_header})
set(SP_VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "SP_VERSION_PATCH ([0-9]*)" _ ${sigpack_header})
set(SP_VERSION_PATCH ${CMAKE_MATCH_1})
message("SIGPACK version: ${SP_VERSION_MAJOR}.${SP_VERSION_MINOR}.${SP_VERSION_PATCH}")

add_library(sigpack INTERFACE)

target_compile_definitions(sigpack INTERFACE HAVE_FFTW) # Add fftw support
target_link_libraries(sigpack INTERFACE armadillo fftw3)
target_include_directories(sigpack INTERFACE dependencies/sigpack)

# mlpack
find_package(MLPACK REQUIRED)

################################################################################
# build
################################################################################
add_library(dsp STATIC)
target_sources(dsp PRIVATE
    src/dsp_error.cpp
    src/pipeline.cpp
    src/filter.cpp
    )
target_include_directories(dsp PUBLIC include)
target_link_libraries(dsp PUBLIC common sigpack cnpy-static z mlpack gomp)
target_compile_options(dsp PUBLIC -Wall -Wextra -fopenmp)

if (DSP_PROFILE)
    target_compile_definitions(dsp PRIVATE DSP_PROFILE)
endif()

################################################################################
# tools
################################################################################
add_executable(qi_training tools/qi_training.cpp)
target_link_libraries(qi_training PUBLIC dsp boost_serialization boost_math_c99)
target_compile_options(qi_training PUBLIC -Wall -Wextra -fopenmp)

################################################################################
# tests
################################################################################
include(CTest)
if(DSP_RUNTESTS)
    add_subdirectory(test)
endif()
