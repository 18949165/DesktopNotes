cmake_minimum_required(VERSION 3.21)
project(StickyNotes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(StandardProjectSettings)
include(CompilerWarnings)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# 第三方
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# GoogleTest
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# UI：ElaWidgetTools (M0.1 暂不启用)
# FetchContent_Declare(qfluentwidgets
#     GIT_REPOSITORY https://github.com/Liniyous/ElaWidgetTools.git
#     GIT_TAG        main)
# FetchContent_MakeAvailable(qfluentwidgets)

enable_testing()
add_subdirectory(src/platform)
add_subdirectory(src/core)
add_subdirectory(src/app)
add_subdirectory(src/ui)
# add_subdirectory(tests)  # M0.2 时启用
