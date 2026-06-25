# Engine/ThirdParty/SPIRV-Cross/CMakeLists.txt
include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# =====================================================
# Core Library (spirv-cross-core)
# =====================================================
set(CORE_SOURCES
    Private/spirv_cross.cpp
    Private/spirv_cfg.cpp
    Private/spirv_parser.cpp
    Private/spirv_cross_parsed_ir.cpp
    Private/spirv_cross_util.cpp
)

add_engine_external_library(spirv-cross-core STATIC ${CORE_SOURCES})

target_include_directories(spirv-cross-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
)

# =====================================================
# Generate Combined Library (spirv-cross-backend) HLSL + GLSL
# =====================================================
# Create a combined library for ease of linking
add_engine_external_library(spirv-cross-backend STATIC
    ${CORE_SOURCES}
    Private/spirv_glsl.cpp
    Private/spirv_hlsl.cpp
)

target_include_directories(spirv-cross-backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
)