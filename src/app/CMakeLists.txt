# src/app/CMakeLists.txt
add_library(app STATIC main.cpp)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
