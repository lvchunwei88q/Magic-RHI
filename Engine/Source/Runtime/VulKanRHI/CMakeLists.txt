include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)
set(VULKAN_ROOT ${THIRD_PARTY_DIR}/VulkanSdk)

add_module_sources(VulKanRHI)

add_engine_library(VulKanRHI SHARED
    ${VulKanRHI_SOURCES}
    ${VulKanRHI_HEADERS}
)

target_include_directories(VulKanRHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can also access it
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects cannot access it
        ${VULKAN_ROOT}/Include # include Vulkan headers
)

target_link_options(VulKanRHI PRIVATE 
    /DEF:${CMAKE_CURRENT_SOURCE_DIR}/Private/VulKanRHI.def
)

target_compile_definitions(VulKanRHI PRIVATE VULKANRHI_EXPORTS)

target_link_libraries(VulKanRHI
    PRIVATE
        ${VULKAN_ROOT}/lib/vulkan-1.lib
)

target_link_libraries(VulKanRHI PRIVATE RHI)
target_link_libraries(VulKanRHI PRIVATE Core)

# Collect all files in the dll directory
file(GLOB VULKAN_DLL_FILES "${VULKAN_ROOT}/dll/*.dll")
file(GLOB VULKAN_JSON_FILES "${VULKAN_ROOT}/dll/*.json")

# Copy all collected files
add_custom_command(TARGET VulKanRHI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${VULKAN_DLL_FILES}
        ${VULKAN_JSON_FILES}
        $<TARGET_FILE_DIR:VulKanRHI>
)