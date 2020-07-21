cmake_minimum_required (VERSION 3.15)
project (libdsp VERSION 0.1.0 LANGUAGES CXX)

################################################################################
# options
################################################################################
# Used for autocompletion in vim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow static library to be included in another lib (used for python wrapping)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

################################################################################
# dependencies
################################################################################
# common
if (TARGET common)
else()
    add_subdirectory(dependencies/libcommon)
endif()

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

################################################################################
# build
################################################################################
add_library(dsp INTERFACE)
target_include_directories(dsp INTERFACE include)
target_link_libraries(dsp INTERFACE common sigpack)
target_compile_options(dsp INTERFACE -Wall -Wextra)

################################################################################
# tests
################################################################################
if (DSP_PROFILE)
target_compile_definitions(dsp INTERFACE DSP_PROFILE)
endif()

include(CTest)
if(DSP_RUNTESTS)
    add_subdirectory(test)
endif()
