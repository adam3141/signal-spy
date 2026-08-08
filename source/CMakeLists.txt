add_subdirectory(signal)

add_library(signal-spy-lib STATIC)

target_sources(signal-spy-lib
    PRIVATE
        ${SIGNAL_SOURCES}
)

target_include_directories(signal-spy-lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)
