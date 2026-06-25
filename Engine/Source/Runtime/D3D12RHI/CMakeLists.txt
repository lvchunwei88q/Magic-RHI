include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)
set(DXC_ROOT ${THIRD_PARTY_DIR}/DirectXShaderCompiler) # DXC root directory

add_module_sources(D3D12RHI)

add_engine_library(D3D12RHI SHARED
    ${D3D12RHI_SOURCES}
    ${D3D12RHI_HEADERS}
)

target_include_directories(D3D12RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects cannot access it
        ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/Public # include RHI headers
        ${DXC_ROOT} # include DXC headers
)

target_link_options(D3D12RHI PRIVATE 
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/D3D12RHI.def
)

target_compile_definitions(D3D12RHI PRIVATE D3D12RHI_EXPORTS)

target_link_libraries(D3D12RHI
    PRIVATE   
        d3d12.lib
        ${DXC_ROOT}/dxcompiler.lib
        dxgi.lib
        dxguid.lib
)

target_link_libraries(D3D12RHI PRIVATE RHI)
target_link_libraries(D3D12RHI PRIVATE Core)
