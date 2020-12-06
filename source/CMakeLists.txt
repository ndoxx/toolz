add_executable(bitdiff)
target_sources(bitdiff
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/bitdiff.cpp"
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


add_executable(thext)
target_sources(thext
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/thext.cpp"
    )
set_target_properties(thext
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_link_libraries(thext
    PRIVATE
        project_options
        project_warnings
        stdc++fs
        kibble # My utility lib
)

cotire(bitdiff)
install(TARGETS bitdiff DESTINATION bin)

cotire(thext)
install(TARGETS thext DESTINATION bin)