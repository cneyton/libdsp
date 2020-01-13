add_library(algo STATIC "")

target_sources(algo
    PRIVATE
    )

target_link_libraries(algo pthread common)

target_include_directories(algo
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
