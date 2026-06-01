include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RHI)

add_engine_library(RHI SHARED
    ${RHI_SOURCES}
    ${RHI_HEADERS}
)

# RHI实现的输出dll文件名
file(GENERATE
    OUTPUT ${CMAKE_BINARY_DIR}/generated/RHIConfig.h
    CONTENT "#pragma once\n#define RHI_D3D11_DLL_FILE \"$<TARGET_FILE_NAME:DirectX11RHI>\"\n#define RHI_D3D12_DLL_FILE \"$<TARGET_FILE_NAME:DirectX12RHI>\""
)
target_include_directories(RHI PUBLIC ${CMAKE_BINARY_DIR}/generated)

target_include_directories(RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(RHI PRIVATE RHI_EXPORTS)

target_link_libraries(RHI PRIVATE Core)
