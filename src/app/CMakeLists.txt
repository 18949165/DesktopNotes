# src/app/CMakeLists.txt
add_library(app STATIC
    src/app_context.cpp
    src/reminder_service.cpp
    src/app_entry.cpp
    src/runtime.cpp
    include/app/reminder_service.h
    include/app/runtime.h
)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PUBLIC core platform ui Qt6::Core Qt6::Gui Qt6::Widgets)

add_executable(StickyNotes main.cpp)
target_link_libraries(StickyNotes PRIVATE app)
