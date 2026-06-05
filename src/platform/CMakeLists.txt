add_library(platform STATIC
    win/filesystem_qt.cpp)
target_include_directories(platform PUBLIC include)
target_link_libraries(platform PUBLIC Qt6::Core)
