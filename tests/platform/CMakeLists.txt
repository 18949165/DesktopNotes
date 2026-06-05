include(GoogleTest)

add_executable(platform_tests
    filesystem_qt_test.cpp)
target_link_libraries(platform_tests
    PRIVATE
        platform
        Qt6::Core
        GTest::gtest_main)
target_include_directories(platform_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
gtest_discover_tests(platform_tests)
