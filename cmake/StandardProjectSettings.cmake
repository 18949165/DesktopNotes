# cmake/StandardProjectSettings.cmake
function(set_project_standards target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)
endfunction()
