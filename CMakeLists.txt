cmake_minimum_required(VERSION 3.15)
project(MagicRHI)

include(${PROJECT_SOURCE_DIR}/Engine/Cmake/common.cmake)
include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# add engine modules
ThirdPartyModule(SPIRV-Cross)

RuntimeModule(Core)
RuntimeModule(VulKanRHI)
RuntimeModule(D3D12RHI)
RuntimeModule(RHI)

# define a function to add RHI tests
function(AddRHITest TARGET_NAME TEST_DIR)
    set(SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR}/${TARGET_NAME}.cpp")
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE RHI Core)
    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

# add executable(HelloTriangleTest Tests/HelloTriangleTest/HelloTriangleTest.cpp)
AddRHITest(HelloTriangleTest "Tests/HelloTriangleTest")
# add executable(TestSwapChain Tests/TestSwapChain/TestSwapChain.cpp)
AddRHITest(TestSwapChain "Tests/TestSwapChain")
# add executable(TestShaderCompile Tests/TestShaderCompile/TestShaderCompile.cpp)
AddRHITest(TestShaderCompile "Tests/TestShaderCompile")
# add ...