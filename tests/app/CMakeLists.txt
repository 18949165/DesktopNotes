add_executable(app_tests reminder_service_test.cpp)
target_include_directories(app_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(app_tests PRIVATE app core platform GTest::gtest_main GTest::gmock Qt6::Core)
gtest_discover_tests(app_tests)
