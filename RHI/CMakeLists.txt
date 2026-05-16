include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RHI)

# 添加库
add_engine_library(RHI SHARED
    ${RHI_SOURCES}
    ${RHI_HEADERS}
)

target_include_directories(RHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

# 定义 DLL 导出宏
target_compile_definitions(RHI PRIVATE RHI_EXPORTS)