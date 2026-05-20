include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(DirectX11RHI)

add_engine_library(DirectX11RHI SHARED
    ${DirectX11RHI_SOURCES}
    ${DirectX11RHI_HEADERS}
)

target_include_directories(DirectX11RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/Public # 包含RHI的头文件
)

target_compile_definitions(DirectX11RHI PRIVATE DIRECTX11RHI_EXPORTS)

target_link_options(DirectX11RHI PRIVATE 
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/DirectX11RHI.def
)

target_link_libraries(DirectX11RHI
    PRIVATE
        RHI           
        d3d11.lib
        dxgi.lib
        dxguid.lib
)

target_link_libraries(DirectX11RHI PRIVATE Core)
