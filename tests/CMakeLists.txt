include(GoogleTest)

add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE GTest::gtest_main)
gtest_discover_tests(smoke_test)

add_subdirectory(platform)
add_subdirectory(core)
# add_subdirectory(app) # 暂时注释，避免网络下载问题
