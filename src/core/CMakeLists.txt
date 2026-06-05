add_library(core STATIC
    src/note_serialize.cpp)
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC Qt6::Core PRIVATE platform)
