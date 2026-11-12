
if(MSVC)
    message(STATUS "🔧 MSVC: Applying MSVC-specific settings...")
    add_compile_options(/wd4819)
    add_compile_options(/wd4828) 
    add_compile_options(/utf-8)

    string(REPLACE "/showIncludes" "" CMAKE_DEPFILE_FLAGS_C "${CMAKE_DEPFILE_FLAGS_C}")
    string(REPLACE "/showIncludes" "" CMAKE_DEPFILE_FLAGS_CXX "${CMAKE_DEPFILE_FLAGS_CXX}")
    message(STATUS "MSVC: /utf-8 enabled")
    # Max object size for MSVC is 2GB, but we can increase it to 4GB with /bigobj
    add_compile_options(/bigobj)
endif()

# Define the third-party libraries directory
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/Engine/ThirdParty)
# Define the content root directory
set(CONTENT_DIR ${CMAKE_SOURCE_DIR}/Engine/Content)

# Set the output directory for runtime files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binary/$<CONFIG>)
# Set the output directory for generated files
set(CMAKE_GENERATE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/engine_mht_generate)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MAGIC_HEADER_TOOL_TARGET "MagicHeaderTool"    CACHE INTERNAL "MagicHeaderTool target name")
set(MAGIC_HEADER_TOOL_RUN    "RunMagicHeaderTool" CACHE INTERNAL "MagicHeaderTool run name")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Write the list of header files into a text file, which will be used for generating reflection code in MHT
set(ENGINE_GENERATE_INFO_DIRECTORY "${CMAKE_GENERATE_OUTPUT_DIRECTORY}/engine_generate_info")
set(HEADER_LIST_FILE "${ENGINE_GENERATE_INFO_DIRECTORY}/engine_headers.buildmeta")
set(ENGINE_COMPILE_COMMANDS_FILE "${ENGINE_GENERATE_INFO_DIRECTORY}/engine_compile_commands.buildmeta")
set(GENERATE_HEADER_LINKPATH "")
file(MAKE_DIRECTORY "${ENGINE_GENERATE_INFO_DIRECTORY}")
file(WRITE "${HEADER_LIST_FILE}" "")

# Windows API use Unicode by default
add_definitions(-DUNICODE -D_UNICODE)

# ============================================================
# 项目版本号定义：MagicToon Engine
# 版本号格式：大版本.功能新增.功能升级.Bug修复 (MAJOR.FEATURE.UPGRADE.BUGFIX)
# 
# 版本分类说明：
#   第一位 (MAJOR)    大版本：    架构级重构、不兼容的 API 变更、重大里程碑发布
#   第二位 (FEATURE)  功能新增：  全新的子系统、模块或重要特性引入
#   第三位 (UPGRADE)  功能升级：  现有功能的扩展、增强、性能优化
#   第四位 (BUGFIX)   Bug修复：   缺陷修复、补丁、小范围调整
#
# 注意：本规则为项目自定义版本规范，与标准 Semver (Major.Minor.Patch) 略有差异
# ============================================================
file(READ "${CMAKE_SOURCE_DIR}/VERSION" PROJECT_VERSION_STRING)
string(STRIP "${PROJECT_VERSION_STRING}" PROJECT_VERSION_STRING)

# 按 . 拆分成列表
string(REPLACE "." ";" VERSION_LIST "${PROJECT_VERSION_STRING}")
list(GET VERSION_LIST 0 MAJOR_VERSION)
list(GET VERSION_LIST 1 FEATURE_VERSION)
list(GET VERSION_LIST 2 UPGRADE_VERSION)
list(GET VERSION_LIST 3 BUGFIX_VERSION)