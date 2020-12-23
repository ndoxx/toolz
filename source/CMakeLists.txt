add_executable(bitdiff)
target_sources(bitdiff
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/bitdiff.cpp"
    )
set_target_properties(bitdiff
    PROPERTIES
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
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_link_libraries(thext
    PRIVATE
        project_options
        project_warnings
        stdc++fs
        kibble # My utility lib
)


add_executable(pencel)
target_sources(pencel
    PRIVATE
        "${CMAKE_SOURCE_DIR}/source/vendor/lodepng/lodepng.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/pencel.cpp"
    )
set_source_files_properties("${CMAKE_SOURCE_DIR}/source/vendor/lodepng/lodepng.cpp" PROPERTIES COMPILE_FLAGS -w)
target_include_directories(pencel
    PRIVATE
        "${CMAKE_SOURCE_DIR}/source/vendor"
    )
set_target_properties(pencel
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_link_libraries(pencel
    PRIVATE
        project_options
        project_warnings
        stdc++fs
        kibble # My utility lib
        png
        z
)

cotire(bitdiff)
install(TARGETS bitdiff DESTINATION bin)

cotire(thext)
install(TARGETS thext DESTINATION bin)

cotire(pencel)
install(TARGETS pencel DESTINATION bin)