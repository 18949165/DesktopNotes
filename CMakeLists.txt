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

# 应用程序元数据
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

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
add_subdirectory(tests)

# NSIS 打包
set(CPACK_PACKAGE_NAME "StickyNotes")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sticky Notes Desktop Application")
set(CPACK_PACKAGE_VENDOR "StickyNotes")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "StickyNotes")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_NSIS_DISPLAY_NAME "StickyNotes")
set(CPACK_NSIS_PACKAGE_NAME "StickyNotes ${VERSION}")
set(CPACK_NSIS_HELP_LINK "https://github.com/your-repo/stickynotes")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/your-repo/stickynotes")
set(CPACK_NSIS_CONTACT "support@example.com")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
