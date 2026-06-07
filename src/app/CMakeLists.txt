# src/app/CMakeLists.txt
add_library(app STATIC
    src/app_context.cpp
    src/runtime.cpp
    include/app/runtime.h
)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PUBLIC core platform ui Qt6::Core Qt6::Gui Qt6::Widgets)

add_executable(StickyNotes main.cpp)
target_link_libraries(StickyNotes PRIVATE app)

# 打包支持：让 StickyNotes.exe 能被 install() 收集
install(TARGETS StickyNotes
    RUNTIME DESTINATION .
    BUNDLE  DESTINATION .
)
