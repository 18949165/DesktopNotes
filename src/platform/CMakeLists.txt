add_library(platform STATIC
    win/filesystem_qt.cpp
    win/system_clock.cpp
    win/hotkey_win.cpp
    win/trayicon_win.cpp)
target_include_directories(platform PUBLIC include)
target_link_libraries(platform PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
