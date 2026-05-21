include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(IO)

# 添加库
add_engine_library(IO SHARED
    ${IO_SOURCES}
    ${IO_HEADERS}
)

target_include_directories(IO
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private # 别的项目不可以访问
)

target_compile_definitions(IO PRIVATE IO_EXPORTS)

target_link_libraries(IO PUBLIC Core)