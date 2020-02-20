set(files
    source_filter_test.cpp
    iir_filter_test.cpp
    #fft_filter_test.cpp
    fd_filter_test.cpp
    correlate_test.cpp
    peaks_test.cpp
    interp_test.cpp
    fhr_filter_test.cpp
    )

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    add_executable(${file_basename} ${file})
    target_link_libraries(${file_basename} dsp)
    target_compile_options(${file_basename} PUBLIC -Werror -Wall -Wextra)
endforeach()
