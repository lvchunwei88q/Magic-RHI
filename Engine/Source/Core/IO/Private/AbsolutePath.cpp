#include "AbsolutePath.h"
#include <windows.h>
#include <filesystem>

namespace IO {

    std::wstring AbsolutePath::GetExecutableDirectory()
    {
        std::filesystem::path exePath = GetExecutablePath();
        return exePath.parent_path().wstring();
    }

    std::wstring AbsolutePath::GetCurrentWorkingDirectory()
    {
        return std::filesystem::current_path().wstring();
    }

    std::wstring AbsolutePath::GetExecutablePath()
    {
        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(NULL, buffer, MAX_PATH);
        if (length == 0 || length == MAX_PATH) {
            return L"";
        }
        return std::filesystem::path(buffer).wstring();
    }

    std::wstring AbsolutePath::GetContentPath()
    {
        std::filesystem::path exePath = GetExecutableDirectory();

        // 可执行文件目录 → 上级 → 上级 → Engine/Content
        std::filesystem::path contentPath = exePath.parent_path()
            .parent_path()
            / L"Engine"
            / L"Content";
        //std::wstring widePath(narrowPath.begin(), narrowPath.end());
        return contentPath;
    }

    std::wstring AbsolutePath::GetScriptPath()
    {
        std::filesystem::path exePath = GetExecutableDirectory();

        // 可执行文件目录 → 上级 → 上级 → Engine/Script
        std::filesystem::path scriptPath = exePath.parent_path()
            .parent_path()
            / L"Engine"
            / L"Script";
        //std::wstring widePath(narrowPath.begin(), narrowPath.end());
        return scriptPath;
    }
}