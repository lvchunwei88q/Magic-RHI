include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(VulKanRHI)

add_engine_library(VulKanRHI SHARED
    ${VulKanRHI_SOURCES}
    ${VulKanRHI_HEADERS}
)

# find Vulkan Sdk package
find_package(Vulkan QUIET REQUIRED)

target_include_directories(VulKanRHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects cannot access it
        ${VULKAN_INCLUDE_DIRS} # include Vulkan headers
)

target_link_options(VulKanRHI PRIVATE 
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/VulKanRHI.def
)

target_compile_definitions(VulKanRHI PRIVATE VULKANRHI_EXPORTS)

target_link_libraries(VulKanRHI PRIVATE Vulkan::Vulkan)

target_link_libraries(VulKanRHI PRIVATE RHI)
target_link_libraries(VulKanRHI PRIVATE Core)
