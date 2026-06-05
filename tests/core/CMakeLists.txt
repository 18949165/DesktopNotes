include(GoogleTest)

add_executable(core_tests
    note_serialize_test.cpp
    markdown_codec_test.cpp)
target_link_libraries(core_tests
    PRIVATE
        core
        Qt6::Core
        GTest::gtest_main)
target_include_directories(core_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
gtest_discover_tests(core_tests)
