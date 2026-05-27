# MagicRHI

本项目使用MSVC工具链开发，并且使用Trae AI辅助，目前处于快速开发期

## 🔧 构建指南

### 环境要求

| 组件                | 版本要求        | 下载链接                                      |
| ----------------- | ----------- | ----------------------------------------- |
| **Visual Studio** | 2022 或 2026 | [下载](https://visualstudio.microsoft.com/) |
| **CMake**         | 3.20 或更高    | [下载](https://cmake.org/download/)         |
| **Git**           | 最新版本        | [下载](https://git-scm.com/)                |
| **Git LFS**       | 最新版本        | [下载](https://git-lfs.github.com/)         |

> ⚠️ **重要**：Git LFS 是必需的，否则无法获取 ThirdParty 文件夹中的大文件。

### 获取源码

```bash
git clone https://github.com/lvchunwei88q/DirectXRHI.git
```

这里也会自动下载 ThirdParty 文件夹中的依赖库。
之后更新仓库时请使用Update.bat脚本，以确保依赖库也能正确更新。

### 构建

在构建之前，请确保已安装 Visual Studio 和 CMake，并且已正确设置环境变量。

```bash
cd DirectXRHI
# 运行 Build 脚本
Build.bat
```

选择Debug或Release版本进行构建，
之后选择使用Ninja或MSBuild生成构建文件，最后Build脚本会自动构建整个项目。
