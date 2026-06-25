cmake_minimum_required(VERSION 3.15)
project(MagicRHI)

include(${PROJECT_SOURCE_DIR}/Engine/Cmake/common.cmake)
include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# add engine modules
ThirdPartyModule(SPIRV-Cross)

RuntimeModule(Core)
RuntimeModule(DirectX11RHI)
RuntimeModule(DirectX12RHI)
RuntimeModule(RHI)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE RHI Core)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
