# MagicRHI

[简体中文](README.md) | [English](README.en.md)

MagicRHI is a C++ RHI (Rendering Hardware Interface) project. It currently includes D3D12 and Vulkan backends. The project is under rapid development, so its APIs and features may change frequently.

## Requirements

| Component | Requirement | Notes |
| --- | --- | --- |
| Operating system | Windows x64 | The project uses the Windows API and the MSVC toolchain |
| Visual Studio | 2022 or 2026 | Install the "Desktop development with C++" workload and a Windows SDK |
| CMake | 3.15 or later | Matches the minimum version declared in the root `CMakeLists.txt` |
| Vulkan SDK | Required | Required for both Debug and Release builds; install it and make sure CMake can find it |
| Git | Any current version | Used to obtain and update the source code |
| Git LFS | Required | Used to obtain `.dll` and `.lib` files tracked by LFS in ThirdParty |
| Ninja | Required when building with Ninja | Not needed when building with Visual Studio/MSBuild |

When building with Ninja, `ninja` and the MSVC compiler `cl` must also be available in the terminal. It is recommended to start the build from an **x64 Native Tools Command Prompt for VS** or the corresponding Developer PowerShell.

The Vulkan SDK is not included in this repository. Both Debug and Release builds include the Vulkan backend, so the SDK must be installed for CMake configuration and building. Download it from the [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home).

## Getting the Source

Install Git LFS and initialize it first:

```powershell
git lfs install
git clone https://github.com/lvchunwei88q/Magic-RHI.git
cd Magic-RHI
```

After cloning, make sure LFS-managed dependencies have been downloaded and are not just pointer files. If files are missing, run the following command from the repository directory:

```powershell
git lfs pull
```

The ThirdParty directory in the repository contains dependencies such as DirectX Shader Compiler and SPIRV-Cross. The Vulkan SDK must be installed separately by the user. To update an existing checkout, you can run `update.bat`; it executes `git pull` and `git lfs pull`. Resolve any local uncommitted changes before updating.

## Building

Run the following command from the repository root:

```powershell
.\build.bat
```

The script will prompt you to select, in order:

1. Build method: `1` for Visual Studio/MSBuild or `2` for Ninja.
2. Build configuration: `1` for Debug or `2` for Release.

The script generates the build files and builds the project. Visual Studio builds use x64. Ninja builds generate `compile_commands.json` and copy it to `out\compile_commands.json`.

You can also configure and build directly with CMake, for example, from a terminal with the MSVC environment set up:

```powershell
cmake -S . -B out\VS_Debug -G "Visual Studio 17 2022" -A x64
cmake --build out\VS_Debug --config Debug
```

## Build Artifacts

Executables and runtime DLLs are output to the directory for the selected configuration under the repository root:

```text
Binary\Debug\
Binary\Release\
```

For example, test programs from a Debug build are located in `Binary\Debug\`. DXC DLLs are copied to the corresponding output directory during the build; Vulkan SDK files and validation layers are not copied.

## Vulkan Runtime Requirements

Debug builds enable the Vulkan validation layer by default. To run a Debug build using the Vulkan backend, install the Vulkan SDK and make sure the Vulkan Loader can find `VK_LAYER_KHRONOS_validation`. Release builds do not enable validation layers and do not require them.

Both Debug and Release Vulkan backends require the system `vulkan-1.dll` (the Vulkan Loader) and a compatible graphics driver at runtime. This DLL is not an SDK-only Debug dependency: the application links against the Vulkan Loader at build time, and the system Vulkan Runtime/graphics driver provides it at runtime. Without a Vulkan Loader or driver, the Vulkan backend is unavailable; D3D12-only use does not require the Vulkan runtime.

## Running the Test Programs

CMake currently defines the following three test executable targets:

| Program | Purpose | Notes |
| --- | --- | --- |
| `HelloTriangleTest.exe` | Hello Triangle rendering example | Uses an available RHI backend; requires a supported GPU and driver |
| `TestSwapChain.exe` | Swap chain and presentation test | Creates a window and runs for about 5 seconds; requires a supported GPU and driver |
| `TestShaderCompile.exe` | Shader compilation test | Reads test shaders from `Tests\TestShaderCompile\Shader` |

After building, run the Debug versions directly from PowerShell at the repository root:

```powershell
.\Binary\Debug\HelloTriangleTest.exe
.\Binary\Debug\TestSwapChain.exe
.\Binary\Debug\TestShaderCompile.exe
```

To run the Release versions, replace `Debug` in the paths with `Release`. These programs are standalone test executables.
