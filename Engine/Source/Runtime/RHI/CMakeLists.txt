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
    CONTENT "#pragma once\n#define RHI_D3D11_DLL_FILE \"$<TARGET_FILE_NAME:D3D11RHI>\"\n#define RHI_D3D12_DLL_FILE \"$<TARGET_FILE_NAME:D3D12RHI>\""
)
target_include_directories(RHI PUBLIC ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/RHI)

target_include_directories(RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${DXC_ROOT} # include DXC headers
)

target_compile_definitions(RHI PRIVATE RHI_EXPORTS)

target_link_libraries(RHI PRIVATE Core)
target_link_libraries(RHI PRIVATE ${DXC_ROOT}/dxcompiler.lib)
