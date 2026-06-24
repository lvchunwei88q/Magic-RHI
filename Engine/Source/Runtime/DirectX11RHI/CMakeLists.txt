include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(D3D11RHI)

add_engine_library(D3D11RHI SHARED
    ${D3D11RHI_SOURCES}
    ${D3D11RHI_HEADERS}
)

target_include_directories(D3D11RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/Public # 包含RHI的头文件
)

target_compile_definitions(D3D11RHI PRIVATE D3D11RHI_EXPORTS)

target_link_options(D3D11RHI PRIVATE 
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/D3D11RHI.def
)

target_link_libraries(D3D11RHI
    PRIVATE   
        d3d11.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib # 使用默认的d3d编译器
)

target_link_libraries(D3D11RHI PRIVATE RHI)
target_link_libraries(D3D11RHI PRIVATE Core)
