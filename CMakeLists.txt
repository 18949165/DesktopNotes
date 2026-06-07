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

# ElaWidgetTools 维护版的 CMakeLists 硬写 QT_SDK_DIR=D:/Qt/5.15.2/msvc2019_64,
# 用环境变量覆盖, 让 Ela 用我们实际的 Qt 路径(6.7.3)。
set(ENV{QT_SDK_DIR} "C:/Qt/6.7.3/msvc2022_64")

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

# UI：ElaWidgetTools
FetchContent_Declare(ElaWidgetTools
    GIT_REPOSITORY https://github.com/Liniyous/ElaWidgetTools.git
    GIT_TAG        main)
FetchContent_GetProperties(ElaWidgetTools)
if(NOT elawidgettools_SOURCE_DIR)
    FetchContent_Populate(ElaWidgetTools)
endif()

# Patch Ela 维护版的硬写检查(FATAL_ERROR 拒 6.7+, 我们已实测 6.7.3 可用)
# Ela 源 CMakeLists 里有 4 条 message(FATAL_ERROR ...): 2 条中文 + 2 条英文,
# 把所有 FATAL_ERROR 在这 4 处降级为 WARNING。
file(READ "${elawidgettools_SOURCE_DIR}/CMakeLists.txt" _ela_cmake)
foreach(_ela_msg IN ITEMS
    "message(FATAL_ERROR \"你当前使用的QT版本不在维护范围内"
    "message(FATAL_ERROR \"The QT version you are currently using is no longer under maintenance"
    "message(FATAL_ERROR \"你当前使用的QT版本为不推荐版本"
    "message(FATAL_ERROR \"The QT version you are currently using is not recommended"
)
    string(REPLACE "${_ela_msg}" "message(WARNING \"[ElaWidgetTools] user-patched: 6.7.3 OK" _ela_cmake "${_ela_cmake}")
endforeach()
file(WRITE "${elawidgettools_SOURCE_DIR}/CMakeLists.txt" "${_ela_cmake}")

add_subdirectory(${elawidgettools_SOURCE_DIR} ${ela_BINARY_DIR} EXCLUDE_FROM_ALL)

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
set(CPACK_NSIS_CONTACT "noreply@example.com")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
