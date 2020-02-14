set(files
    source_filter_test.cpp
    iir_filter_test.cpp
    fft_filter_test.cpp
    )
foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    add_executable(${file_basename} ${file})
    target_link_libraries(${file_basename} dsp)
endforeach()
