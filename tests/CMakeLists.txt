enable_testing()

add_subdirectory(signal)

add_executable(signal-spy-tests)

target_sources(signal-spy-tests
    PRIVATE
        ${TEST_SOURCES}
)

target_link_libraries(signal-spy-tests
    PRIVATE
        signal-spy-lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(signal-spy-tests)
