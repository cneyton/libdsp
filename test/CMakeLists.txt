set(files
    correlate_test.cpp
    peaks_test.cpp
    interp_test.cpp
    link_test.cpp
    source_filter_test.cpp
    iir_filter_test.cpp
    #fft_filter_test.cpp
    fd_filter_test.cpp
    fhr_filter_test.cpp
    buffer_filter_test.cpp
    roll_filter_test.cpp
    tee_filter_test.cpp
    #full_pipeline_test.cpp
    #arma_test.cpp
    )

foreach(file ${files})
    get_filename_component(target_name ${file} NAME_WE)
    add_executable(${target_name} ${file})
    target_link_libraries(${target_name} dsp)
    target_compile_options(${target_name} PUBLIC -Werror -Wall -Wextra -g -fno-inline)
    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()

#add_test(NAME full_pipeline_callgrind COMMAND valgrind --tool=callgrind test/full_pipeline_test)
