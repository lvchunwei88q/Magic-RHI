include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(DirectX12RHI)

add_engine_library(DirectX12RHI SHARED
    ${DirectX12RHI_SOURCES}
    ${DirectX12RHI_HEADERS}
)

target_include_directories(DirectX12RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/Public # 包含RHI的头文件
)

target_link_options(DirectX12RHI PRIVATE 
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/DirectX12RHI.def
)

target_compile_definitions(DirectX12RHI PRIVATE DIRECTX12RHI_EXPORTS)

target_link_libraries(DirectX12RHI
    PRIVATE
        RHI           
        d3d12.lib
        dxgi.lib
        dxguid.lib
)

target_link_libraries(DirectX12RHI PRIVATE Core)
