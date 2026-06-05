add_library(core STATIC
    src/note_serialize.cpp
    src/markdown_codec.cpp
    src/file_notestore.cpp
    src/settings.cpp)
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC Qt6::Core PRIVATE platform)
