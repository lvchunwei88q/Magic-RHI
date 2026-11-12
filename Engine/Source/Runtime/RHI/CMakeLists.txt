include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)
set(DXC_ROOT ${THIRD_PARTY_DIR}/DirectXShaderCompiler)

add_module_sources(RHI)

add_engine_library(RHI SHARED
    ${RHI_SOURCES}
    ${RHI_HEADERS}
)

# output RHI.modules.generated.h file
set(RHI_MODULES_CONTENT "#pragma once\n")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # The D3D12 module only supports Windows
    string(APPEND RHI_MODULES_CONTENT
        "#define RHI_D3D12_DLL_FILE \"$<TARGET_FILE_NAME:D3D12RHI>\"\n")
else()
    string(APPEND RHI_MODULES_CONTENT
        "#define RHI_D3D12_DLL_FILE \"\"\n")
endif()

string(APPEND RHI_MODULES_CONTENT
    "#define RHI_VULKAN_DLL_FILE \"$<TARGET_FILE_NAME:VulKanRHI>\"\n")

file(GENERATE
    OUTPUT ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/RHI/RHI.modules.generated.h
    CONTENT "${RHI_MODULES_CONTENT}"
)

target_include_directories(RHI PUBLIC ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/RHI)

target_include_directories(RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects cannot access it
        ${DXC_ROOT} # include DXC headers
)

target_compile_definitions(RHI PRIVATE RHI_EXPORTS)

target_link_libraries(RHI PRIVATE Core)
# SPI SPIRV-Cross
target_link_libraries(RHI PRIVATE spirv-cross-core)
target_link_libraries(RHI PRIVATE spirv-cross-backend)
# dx shader compiler
if (WIN32)
    target_link_libraries(RHI PRIVATE ${DXC_ROOT}/lib/win64/dxcompiler.lib)
elseif(UNIX)
    target_link_libraries(RHI PRIVATE ${DXC_ROOT}/lib/linux/libdxcompiler.so)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if (WIN32)
# Collect all files in the dll directory
file(GLOB DXC_LIB_FILES "${DXC_ROOT}/lib/win64/*.dll")
elseif(UNIX)
# Collect all files in the so directory
file(GLOB DXC_LIB_FILES "${DXC_ROOT}/lib/linux/*.so")
endif()

# Copy all collected files
add_custom_command(TARGET RHI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DXC_LIB_FILES}
        $<TARGET_FILE_DIR:RHI>
)
