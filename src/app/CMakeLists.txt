# src/app/CMakeLists.txt
add_library(app STATIC
    main.cpp
    src/app_context.cpp
    src/reminder_service.cpp
    src/app_entry.cpp
    include/app/reminder_service.h
)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PUBLIC core platform Qt6::Core Qt6::Gui Qt6::Widgets)
