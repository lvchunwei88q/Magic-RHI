include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)
set(DXC_ROOT ${THIRD_PARTY_DIR}/DirectXShaderCompiler) # DXC的根目录

add_module_sources(D3D12RHI)

add_engine_library(D3D12RHI SHARED
    ${D3D12RHI_SOURCES}
    ${D3D12RHI_HEADERS}
)

target_include_directories(D3D12RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/Public # 包含RHI的头文件
        ${DXC_ROOT} # 包含DXC的头文件
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

# 复制DXC的dll文件到构建目录
add_custom_command(TARGET D3D12RHI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DXC_ROOT}/dll/dxcompiler.dll
        $<TARGET_FILE_DIR:D3D12RHI>
)