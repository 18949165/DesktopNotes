include(GoogleTest)

add_executable(ui_tests
    hotkey_edit_test.cpp)
target_link_libraries(ui_tests
    PRIVATE
        ui
        platform
        core
        Qt6::Core
        Qt6::Widgets
        GTest::gtest)
target_include_directories(ui_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/platform/include
    ${CMAKE_SOURCE_DIR}/src/core/include
    ${CMAKE_SOURCE_DIR}/src/ui/include
    ${CMAKE_SOURCE_DIR}/tests)
gtest_discover_tests(ui_tests)
