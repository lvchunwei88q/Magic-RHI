# MagicRHI

[简体中文](README.md) | [English](README.en.md)

MagicRHI 是一个使用 C++ 开发的 RHI（渲染硬件接口）项目，当前包含 D3D12 和 Vulkan 后端。项目处于快速开发期，接口和功能可能持续变化。

## 环境要求

| 组件 | 要求 | 说明 |
| --- | --- | --- |
| 操作系统 | Windows x64 | 项目使用 Windows API 和 MSVC 工具链 |
| Visual Studio | 2022 或 2026 | 安装“使用 C++ 的桌面开发”工作负载及 Windows SDK |
| CMake | 3.15 或更高 | 与根目录 `CMakeLists.txt` 声明的最低版本一致 |
| Vulkan SDK | 必需 | Debug 和 Release 构建都需要；请安装 Vulkan SDK，并确保 CMake 能找到它 |
| Git | 可用版本 | 用于获取和更新源码 |
| Git LFS | 必需 | 用于获取 ThirdParty 中受 LFS 管理的 `.dll` 和 `.lib` 文件 |
| Ninja | 使用 Ninja 构建时需要 | 使用 Visual Studio/MSBuild 构建时不需要 |

使用 Ninja 构建时，还需要在终端中能够找到 `ninja` 和 MSVC 编译器 `cl`。建议从 **x64 Native Tools Command Prompt for VS** 或相应的开发者 PowerShell 启动构建。

Vulkan SDK 不包含在仓库中。Debug 和 Release 都会构建 Vulkan 后端，因此配置和构建时都需要安装 SDK。可从 [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home) 获取。

## 获取源码

先安装 Git LFS 并初始化：

```powershell
git lfs install
git clone https://github.com/lvchunwei88q/Magic-RHI.git
cd Magic-RHI
```

克隆后，确认 LFS 管理的依赖文件已下载，而不是仅显示为指针文件。若文件缺失，可在仓库目录运行：

```powershell
git lfs pull
```

仓库内的 ThirdParty 目录包含 DirectX Shader Compiler 和 SPIRV-Cross 等依赖；Vulkan SDK 需由用户自行安装。更新已有源码时可运行 `update.bat`；该脚本会执行 `git pull` 和 `git lfs pull`。更新前请先处理本地未提交的修改。

## 构建

在仓库根目录运行：

```powershell
.\build.bat
```

脚本会依次询问：

1. 构建方式：`1` 使用 Visual Studio/MSBuild，`2` 使用 Ninja。
2. 构建配置：`1` 使用 Debug，`2` 使用 Release。

脚本会生成构建文件并构建项目。Visual Studio 构建使用 x64；Ninja 构建会生成 `compile_commands.json`，并复制到 `out\compile_commands.json`。

也可以直接用 CMake 配置和构建，例如在具备 MSVC 环境的终端中：

```powershell
cmake -S . -B out\VS_Debug -G "Visual Studio 17 2022" -A x64
cmake --build out\VS_Debug --config Debug
```

## 构建产物

可执行文件和运行时 DLL 输出到仓库根目录下对应配置的目录：

```text
Binary\Debug\
Binary\Release\
```

例如，Debug 构建的测试程序位于 `Binary\Debug\`。DXC DLL 会在构建时复制到相应的输出目录；Vulkan SDK 文件和验证层不会随构建产物复制。

## Vulkan 运行要求

Debug 版默认启用 Vulkan 验证层。运行 Debug 版并使用 Vulkan 后端时，需要安装 Vulkan SDK，并确保 `VK_LAYER_KHRONOS_validation` 验证层可被 Vulkan Loader 找到。Release 版不启用验证层，不要求安装验证层。

Debug 和 Release 的 Vulkan 后端运行时都需要系统提供 `vulkan-1.dll`（Vulkan Loader）及兼容的显卡驱动。该 DLL 不是只在 Debug 下需要的 SDK 文件：构建时会链接 Vulkan Loader，运行时由系统的 Vulkan Runtime/显卡驱动提供。如果目标机器没有 Vulkan Loader 或驱动，Vulkan 后端不可用；仅使用 D3D12 时不需要 Vulkan SDK 运行时。

## 运行测试程序

目前 CMake 配置了以下三个测试可执行目标：

| 程序 | 用途 | 运行提示 |
| --- | --- | --- |
| `HelloTriangleTest.exe` | Hello Triangle 渲染示例 | 会使用可用的 RHI 后端；需要支持的显卡和驱动 |
| `TestSwapChain.exe` | 交换链和呈现测试 | 会创建窗口并运行约 5 秒；需要支持的显卡和驱动 |
| `TestShaderCompile.exe` | 着色器编译测试 | 会读取 `Tests\TestShaderCompile\Shader` 下的测试着色器 |

构建完成后，可在仓库根目录通过 PowerShell 直接运行 Debug 版本：

```powershell
.\Binary\Debug\HelloTriangleTest.exe
.\Binary\Debug\TestSwapChain.exe
.\Binary\Debug\TestShaderCompile.exe
```

运行 Release 版本时，将路径中的 `Debug` 替换为 `Release`。这些程序是独立的测试可执行文件。
