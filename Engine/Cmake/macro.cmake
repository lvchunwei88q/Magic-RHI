macro(EditorModule)
    add_subdirectory("Engine/Source/Editor/${ARGV}")
endmacro()
macro(RuntimeModule)
    add_subdirectory("Engine/Source/Runtime/${ARGV}")
endmacro()

macro(ThirdPartyModule)
    add_subdirectory("Engine/ThirdParty/${ARGV}")
endmacro()

macro(add_module_sources ModuleName)
file(GLOB_RECURSE ${ModuleName}_SOURCES 
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.cc"
    "*.cxx"
    "*.c"
)

file(GLOB_RECURSE ${ModuleName}_HEADERS 
    CONFIGURE_DEPENDS
    "*.h"
    "*.hpp"
)
endmacro()

# 给模块的前缀
function(add_engine_library name)
    add_library(${name} ${ARGN})
    set_target_properties(${name} PROPERTIES 
        PREFIX "Magic-"
    )
endfunction()