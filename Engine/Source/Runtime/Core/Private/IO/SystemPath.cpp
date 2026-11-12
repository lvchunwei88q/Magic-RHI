#include "IO.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <codecvt>
#include <locale>
#endif

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
namespace IO {

    namespace {
        fs::path ToFilesystemPath(const std::wstring& path)
        {
#ifdef _WIN32
            return fs::path(path);
#else
            return fs::path(ToNarrowString(path));
#endif
        }

        std::wstring ToWidePath(const fs::path& path)
        {
#ifdef _WIN32
            return path.wstring();
#else
            return ToWideString(path.native());
#endif
        }
    }

    std::wstring AbsolutePath::GetExecutablePath()
    {
        // if the executable path has already been cached, return it directly
        if (!ExecutablePath.empty()) {
            return ExecutablePath;
        }

#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(NULL, buffer, MAX_PATH);
        if (length == 0 || length == MAX_PATH) {
            return L"";
        }

        ExecutablePath = buffer;
#else
        std::vector<char> buffer(1024);
        ssize_t length;
        while (true) {
            length = readlink("/proc/self/exe", buffer.data(), buffer.size());
            if (length < 0) {
                return L"";
            }
            if (static_cast<size_t>(length) != buffer.size()) {
                break;
            }
            buffer.resize(buffer.size() * 2);
        }

        ExecutablePath = ToWideString(std::string(buffer.data(), static_cast<size_t>(length)));
#endif
        return ExecutablePath;
    }

    std::wstring AbsolutePath::GetExecutableDirectory()
    {
        if (!ExecutableDirectory.empty()) {
            return ExecutableDirectory;
        }

        fs::path exePath = ToFilesystemPath(GetExecutablePath());
        ExecutableDirectory = ToWidePath(exePath.parent_path());
        return ExecutableDirectory;
    }

    WorkingDirectory AbsolutePath::GetCurrentWorkingDirectory()
    {
        if (!CurrentWorkingDirectory.empty()) {
            return WorkingDirectory(CurrentWorkingDirectory);
        }

        CurrentWorkingDirectory = ToWidePath(fs::current_path());
        return WorkingDirectory(CurrentWorkingDirectory);
    }

    ContentDirectory AbsolutePath::GetContentDirectory()
    {
        if (!ContentPath.empty()) {
            return ContentDirectory(ContentPath);
        }

        fs::path exeDir = ToFilesystemPath(GetExecutableDirectory());

        // Executable file directory → Up one level → Up one level → Engine/Content
        fs::path fs_contentPath = exeDir.parent_path()
            .parent_path()
            / "Engine"
            / "Content";

        ContentPath = ToWidePath(fs_contentPath);
        return ContentDirectory(ContentPath);
    }

    ScriptDirectory AbsolutePath::GetScriptDirectory()
    {
        if (!ScriptPath.empty()) {
            return ScriptDirectory(ScriptPath);
        }

        fs::path exeDir = ToFilesystemPath(GetExecutableDirectory());

        // Executable file directory → Up one level → Up one level → Engine/Script
        fs::path fs_scriptPath = exeDir.parent_path()
            .parent_path()
            / "Engine"
            / "Script";

        ScriptPath = ToWidePath(fs_scriptPath);
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

#ifdef _WIN32
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), NULL, 0);
        if (size_needed == 0) return std::wstring();
        std::wstring wide(size_needed, 0);
        if (MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), &wide[0], size_needed) == 0) {
            return std::wstring();
        }
        return wide;
#else
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(narrow);
#endif
    }

    std::string ToNarrowString(const std::wstring& wide) {
        if (wide.empty()) return std::string();

#ifdef _WIN32
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), NULL, 0, NULL, NULL);
        if (size_needed == 0) return std::string();
        std::string narrow(size_needed, 0);
        if (WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), &narrow[0], size_needed, NULL, NULL) == 0) {
            return std::string();
        }
        return narrow;
#else
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wide);
#endif
    }
}