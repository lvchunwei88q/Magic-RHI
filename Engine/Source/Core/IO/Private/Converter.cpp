#include "Converter.h"
#include <windows.h>

namespace IO {
    std::wstring Converter::ToWideString(const std::string& narrow) {
        if (narrow.empty()) return std::wstring();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), NULL, 0);
        std::wstring wide(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), &wide[0], size_needed);
        return wide;
    }

    std::string Converter::ToNarrowString(const std::wstring& wide) {
        if (wide.empty()) return std::string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), NULL, 0, NULL, NULL);
        std::string narrow(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), &narrow[0], size_needed, NULL, NULL);
        return narrow;
    }
}