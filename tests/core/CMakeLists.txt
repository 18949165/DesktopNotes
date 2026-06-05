include(GoogleTest)

add_executable(core_tests
    note_serialize_test.cpp
    markdown_codec_test.cpp
    file_notestore_test.cpp
    settings_test.cpp)
target_link_libraries(core_tests
    PRIVATE
        core
        platform
        Qt6::Core
        GTest::gtest_main)
target_include_directories(core_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/platform/include
    ${CMAKE_SOURCE_DIR}/tests)
gtest_discover_tests(core_tests)
