add_subdirectory(signal)
add_subdirectory(ui)

add_library(signal-spy-lib STATIC)

target_sources(signal-spy-lib
    PRIVATE
        ${SIGNAL_SOURCES}
        ${UI_SOURCES}
)

target_include_directories(signal-spy-lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(signal-spy-lib
    PUBLIC
        imgui
)

