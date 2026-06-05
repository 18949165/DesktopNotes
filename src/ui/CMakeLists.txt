# src/ui/CMakeLists.txt
add_library(ui STATIC
    src/note_editor.cpp
    src/mainwindow.cpp
    src/settings_dialog.cpp
    src/stickywindow.cpp
    include/ui/note_editor.h
    include/ui/mainwindow.h
    include/ui/settings_dialog.h
    include/ui/stickywindow.h
)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ui PUBLIC core app platform Qt6::Widgets ElaWidgetTools)
