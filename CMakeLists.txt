cmake_minimum_required(VERSION 3.15)
project(MagicRHI)

include(${PROJECT_SOURCE_DIR}/Engine/Cmake/common.cmake)
include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# add engine modules
ThirdPartyModule(SPIRV-Cross)

RuntimeModule(Core)
RuntimeModule(VulKanRHI)
RuntimeModule(D3D11RHI)
RuntimeModule(D3D12RHI)
RuntimeModule(RHI)

# add executable(HelloTriangleTest Tests/HelloTriangleTest/main.cpp)
add_executable(HelloTriangleTest Tests/HelloTriangleTest/main.cpp)
target_link_libraries(HelloTriangleTest PRIVATE RHI Core)
target_include_directories(HelloTriangleTest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# add ...