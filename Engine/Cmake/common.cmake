# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if(MSVC)
    message(STATUS "🔧 MSVC: Applying MSVC-specific settings...")
    add_compile_options(/wd4819)
    add_compile_options(/wd4828) 
    add_compile_options(/utf-8)

    string(REPLACE "/showIncludes" "" CMAKE_DEPFILE_FLAGS_C "${CMAKE_DEPFILE_FLAGS_C}")
    string(REPLACE "/showIncludes" "" CMAKE_DEPFILE_FLAGS_CXX "${CMAKE_DEPFILE_FLAGS_CXX}")
    message(STATUS "MSVC: /utf-8 enabled")
    # 更改最大数
    add_compile_options(/bigobj)
endif()

# 定义第三方库的根目录
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/Engine/Thirdparty)
# 定义内容根目录
set(CONTENT_DIR ${CMAKE_SOURCE_DIR}/Engine/Content)

# 设置输出目录为项目根目录下的 /Binary/BUILD_TYPE
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binary/$<CONFIG>)
# 设置生成文件的输出目录
set(CMAKE_GENERATE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/Generate)

set(MAGIC_HEADER_TOOL_TARGET "MagicHeaderTool"    CACHE INTERNAL "MagicHeaderTool target name")
set(MAGIC_HEADER_TOOL_RUN    "RunMagicHeaderTool" CACHE INTERNAL "MagicHeaderTool run name")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 写入所有头文件列表的文件路径
set(HEADER_LIST_FILE "${CMAKE_GENERATE_OUTPUT_DIRECTORY}/engine_generate_info/engine_headers.buildmeta")
set(GENERATE_HEADER_MODEL "")
file(WRITE ${HEADER_LIST_FILE} "")

# Windows API 用宽字符版本
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