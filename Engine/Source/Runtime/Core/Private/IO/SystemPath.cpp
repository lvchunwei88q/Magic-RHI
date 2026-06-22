#include "IO.h"
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;
namespace IO {

    std::wstring AbsolutePath::GetExecutablePath()
    {
        // 如果已缓存，直接返回
        if (!ExecutablePath.empty()) {
            return ExecutablePath;
        }

        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(NULL, buffer, MAX_PATH);
        if (length == 0 || length == MAX_PATH) {
            return L"";
        }

        ExecutablePath = fs::path(buffer).wstring();
        return ExecutablePath;
    }

    std::wstring AbsolutePath::GetExecutableDirectory()
    {
        if (!ExecutableDirectory.empty()) {
            return ExecutableDirectory;
        }

        fs::path exePath(GetExecutablePath());
        ExecutableDirectory = exePath.parent_path().wstring();
        return ExecutableDirectory;
    }

    WorkingDirectory AbsolutePath::GetCurrentWorkingDirectory()
    {
        if (!CurrentWorkingDirectory.empty()) {
            return WorkingDirectory(CurrentWorkingDirectory);
        }

        CurrentWorkingDirectory = fs::current_path().wstring();
        return WorkingDirectory(CurrentWorkingDirectory);
    }

    ContentDirectory AbsolutePath::GetContentDirectory()
    {
        if (!ContentPath.empty()) {
            return ContentDirectory(ContentPath);
        }

        fs::path exeDir(GetExecutableDirectory());

        // 可执行文件目录 → 上级 → 上级 → Engine/Content
        fs::path fs_contentPath = exeDir.parent_path()
            .parent_path()
            / L"Engine"
            / L"Content";

        ContentPath = fs_contentPath.wstring();
        return ContentDirectory(ContentPath);
    }

    ScriptDirectory AbsolutePath::GetScriptDirectory()
    {
        if (!ScriptPath.empty()) {
            return ScriptDirectory(ScriptPath);
        }

        fs::path exeDir(GetExecutableDirectory());

        // 可执行文件目录 → 上级 → 上级 → Engine/Script
        fs::path fs_scriptPath = exeDir.parent_path()
            .parent_path()
            / L"Engine"
            / L"Script";

        ScriptPath = fs_scriptPath.wstring();
        return ScriptDirectory(ScriptPath);
    }

    void AbsolutePath::ClearCache()
    {
        ExecutableDirectory.clear();
        CurrentWorkingDirectory.clear();
        ExecutablePath.clear();
        ContentPath.clear();
        ScriptPath.clear();
    }

    std::wstring ToWideString(const std::string& narrow) {
        if (narrow.empty()) return std::wstring();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), NULL, 0);
        std::wstring wide(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), &wide[0], size_needed);
        return wide;
    }

    std::string ToNarrowString(const std::wstring& wide) {
        if (wide.empty()) return std::string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), NULL, 0, NULL, NULL);
        std::string narrow(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), &narrow[0], size_needed, NULL, NULL);
        return narrow;
    }
}