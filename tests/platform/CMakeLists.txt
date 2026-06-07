include(GoogleTest)

add_executable(platform_tests
    filesystem_qt_test.cpp
    clock_test.cpp
    hotkey_test.cpp
    trayicon_test.cpp
    ${CMAKE_SOURCE_DIR}/tests/platform/mock_clock.h
    ${CMAKE_SOURCE_DIR}/tests/platform/mocks.h)
target_link_libraries(platform_tests
    PRIVATE
        platform
        Qt6::Core
        Qt6::Gui
        GTest::gmock_main)
target_include_directories(platform_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
gtest_discover_tests(platform_tests)
