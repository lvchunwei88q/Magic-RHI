include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)
set(DXC_ROOT ${THIRD_PARTY_DIR}/DirectXShaderCompiler)

add_module_sources(RHI)

add_engine_library(RHI SHARED
    ${RHI_SOURCES}
    ${RHI_HEADERS}
)

# output RHI.modules.generated.h file
file(GENERATE
    OUTPUT ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/RHI/RHI.modules.generated.h
    CONTENT "#pragma once
            \n#define RHI_D3D11_DLL_FILE \"$<TARGET_FILE_NAME:D3D11RHI>\"
            \n#define RHI_D3D12_DLL_FILE \"$<TARGET_FILE_NAME:D3D12RHI>\"
            \n#define RHI_VULKAN_DLL_FILE \"$<TARGET_FILE_NAME:VulKanRHI>\""
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
target_link_libraries(RHI PRIVATE ${DXC_ROOT}/dxcompiler.lib)

# Collect all files in the dll directory
file(GLOB DXC_DLL_FILES "${DXC_ROOT}/dll/*.dll")

# Copy all collected files
add_custom_command(TARGET RHI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DXC_DLL_FILES}
        $<TARGET_FILE_DIR:RHI>
)
