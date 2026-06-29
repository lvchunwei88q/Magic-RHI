include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(D3D11RHI)

add_engine_library(D3D11RHI SHARED
    ${D3D11RHI_SOURCES}
    ${D3D11RHI_HEADERS}
)

target_include_directories(D3D11RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects cannot access it
        ${CMAKE_CURRENT_SOURCE_DIR}/../RHI/Public # include RHI headers
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
        d3dcompiler.lib # use default d3d compiler
)

# SPI SPIRV-Cross
target_link_libraries(D3D11RHI PRIVATE spirv-cross-core)
target_link_libraries(D3D11RHI PRIVATE spirv-cross-backend)

target_link_libraries(D3D11RHI PRIVATE RHI)
target_link_libraries(D3D11RHI PRIVATE Core)
