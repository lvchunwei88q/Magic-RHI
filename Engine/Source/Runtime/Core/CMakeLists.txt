include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(CORE)

# add library
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
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too
        ${THIRD_PARTY_DIR}

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
)

target_compile_definitions(Core PRIVATE CORE_EXPORTS)