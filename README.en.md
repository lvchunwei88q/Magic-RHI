# MagicRHI

[简体中文](README.md) | [English](README.en.md)

MagicRHI is a C++ RHI (Rendering Hardware Interface) project. It currently includes D3D12 and Vulkan backends. The project is under rapid development, so its APIs and features may change frequently.

## Requirements

| Component | Requirement | Notes |
| --- | --- | --- |
| Operating system | Windows x64 | The project uses the Windows API and the MSVC toolchain |
| Visual Studio | 2022 or 2026 | Install the "Desktop development with C++" workload and a Windows SDK |
| CMake | 3.15 or later | Matches the minimum version declared in the root `CMakeLists.txt` |
| Git | Any current version | Used to obtain and update the source code |
| Git LFS | Required | Used to obtain `.dll` and `.lib` files tracked by LFS in ThirdParty |
| Ninja | Required when building with Ninja | Not needed when building with Visual Studio/MSBuild |

When building with Ninja, `ninja` and the MSVC compiler `cl` must also be available in the terminal. It is recommended to start the build from an **x64 Native Tools Command Prompt for VS** or the corresponding Developer PowerShell.

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

The ThirdParty directory in the repository contains dependencies required by the project, including Vulkan SDK, DirectX Shader Compiler, and SPIRV-Cross. To update an existing checkout, you can run `update.bat`; it executes `git pull` and `git lfs pull`. Resolve any local uncommitted changes before updating.

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

For example, test programs from a Debug build are located in `Binary\Debug\`. Vulkan runtime files and DXC DLLs are copied to the corresponding output directory during the build.

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
