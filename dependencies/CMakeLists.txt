#####
# libcommon
#####
if (TARGET common)
else()
    add_subdirectory(libcommon)
endif()

#####
# sigpack
#####
# Get version number
file(READ sigpack/sigpack.h sigpack_header)
string(REGEX MATCH "SP_VERSION_MAJOR ([0-9]*)" _ ${sigpack_header})
set(SP_VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "SP_VERSION_MINOR ([0-9]*)" _ ${sigpack_header})
set(SP_VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "SP_VERSION_PATCH ([0-9]*)" _ ${sigpack_header})
set(SP_VERSION_PATCH ${CMAKE_MATCH_1})
message("SIGPACK version: ${SP_VERSION_MAJOR}.${SP_VERSION_MINOR}.${SP_VERSION_PATCH}")

add_library(sigpack INTERFACE)

# Add fftw support
target_compile_definitions(sigpack INTERFACE HAVE_FFTW)

target_link_libraries(sigpack INTERFACE armadillo fftw3)
target_include_directories(sigpack INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sigpack)

#####
# mlpack
#####
