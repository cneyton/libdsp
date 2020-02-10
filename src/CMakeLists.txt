target_sources(dsp
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source_filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sink_filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/link.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/chunk.cpp
    )
