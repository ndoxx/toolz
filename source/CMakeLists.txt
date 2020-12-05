add_executable(bitdiff)
target_sources(bitdiff
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/bitdiff.cpp"
    )
target_include_directories(bitdiff
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

set_target_properties(bitdiff
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_link_libraries(bitdiff
    PRIVATE
        project_options
        project_warnings
        stdc++fs
        kibble # My utility lib
)

cotire(bitdiff)