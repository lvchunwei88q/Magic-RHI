include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(CORE)

# 添加库
add_engine_library(Core SHARED
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_compile_definitions(Core PRIVATE
    MAGICTOON_VERSION="${PROJECT_VERSION_STRING}"
    MAGICTOON_VERSION_MAJOR=${MAJOR_VERSION}
    MAGICTOON_VERSION_FEATURE=${FEATURE_VERSION}
    MAGICTOON_VERSION_UPGRADE=${UPGRADE_VERSION}
    MAGICTOON_VERSION_BUGFIX=${BUGFIX_VERSION}
)

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(Core PRIVATE CORE_EXPORTS)