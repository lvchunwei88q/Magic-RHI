#include "IO.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;
namespace IO {
    namespace {
        void WriteAllBytesImpl(const std::wstring& path, const void* data, size_t size) {
            fs::path p(path);
            std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to write file: " + ToNarrowString(path));
            }
            if (size > 0) {
                file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
                if (!file) {
                    throw std::runtime_error("Failed to write data to file: " + ToNarrowString(path));
                }
            }
        }

        void AppendBytesImpl(const std::wstring& path, const void* data, size_t size) {
            fs::path p(path);
            std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::app);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to append to file: " + ToNarrowString(path));
            }
            if (size > 0) {
                file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
                if (!file) {
                    throw std::runtime_error("Failed to append data to file: " + ToNarrowString(path));
                }
            }
        }
    } // namespace

    std::string ReadAllText(const std::wstring& path) {
        fs::path p(path);
        std::ifstream file(p, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + ToNarrowString(path));
        }
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }

    std::vector<char> ReadAllCharBytes(const std::wstring& path) {
        fs::path p(path);
        std::ifstream file(p, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + ToNarrowString(path));
        }
        std::vector<char> content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }

    std::vector<uint8_t> ReadAllUINT8Bytes(const std::wstring& path) {
        std::vector<char> Content = ReadAllCharBytes(path);
        std::vector<uint8_t> uint8_content(Content.begin(), Content.end());
        return uint8_content;
    }

    BinaryReader ReadAllBinaryReaderBytes(const std::wstring& path)
    {
        std::vector<char> Content = ReadAllCharBytes(path);
        std::vector<uint8_t> uint8_content(Content.begin(), Content.end());
        return BinaryReader(uint8_content);
    }

    void WriteAllText(const std::wstring& path, const std::string& content) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + ToNarrowString(path));
        }
        file.write(content.data(), content.size());
    }

    void WriteAllBytes(const std::wstring& path, const std::vector<char>& data) {
        WriteAllBytesImpl(path, data.data(), data.size());
    }

    void WriteAllBytes(const std::wstring& path, const std::vector<uint8_t>& data) {
        WriteAllBytesImpl(path, data.data(), data.size());
    }

    void WriteAllBytes(const std::wstring& path, const BinaryWriter& data) {
        WriteAllBytesImpl(path, data.DataU8(), data.GetWrittenSize());
    }

    void WriteAllBytes(const std::wstring& path, const uint8_t* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("WriteAllBytes: null data pointer");
        }
        WriteAllBytesImpl(path, data, size);
    }

    void WriteAllBytes(const std::wstring& path, const char* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("WriteAllBytes: null data pointer");
        }
        WriteAllBytesImpl(path, data, size);
    }

    void AppendText(const std::wstring& path, const std::string& content) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to append to file: " + ToNarrowString(path));
        }
        file.write(content.data(), content.size());
    }

    void AppendBytes(const std::wstring& path, const std::vector<char>& data) {
        AppendBytesImpl(path, data.data(), data.size());
    }

    void AppendBytes(const std::wstring& path, const std::vector<uint8_t>& data) {
        AppendBytesImpl(path, data.data(), data.size());
    }

    void AppendBytes(const std::wstring& path, const BinaryWriter& data) {
        AppendBytesImpl(path, data.DataU8(), data.GetWrittenSize());
    }

    void AppendBytes(const std::wstring& path, const uint8_t* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("AppendBytes: null data pointer");
        }
        AppendBytesImpl(path, data, size);
    }

    void AppendBytes(const std::wstring& path, const char* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("AppendBytes: null data pointer");
        }
        AppendBytesImpl(path, data, size);
    }

    bool DeleteToFile(const std::wstring& path) {
        fs::path p(path);
        if (fs::is_regular_file(p)) {
            return fs::remove(p);
        }
        return false;
    }

    bool DeleteToDirectory(const std::wstring& path) {
        fs::path p(path);
        std::error_code ec;

        if (fs::is_directory(p)) {
            fs::remove_all(p, ec);  // remove all content
            return !ec;
        }
        return false;  // Not a folder, return false
    }

    bool CreateFile(const std::wstring& path) {
        fs::path p(path);
        if (fs::exists(p)) {
            // Update Last Modified Time
            fs::last_write_time(p, fs::file_time_type::clock::now());
            return true;
        }
        std::ofstream file(p, std::ios::out | std::ios::binary);
        return file.is_open();
    }

    bool MoveFile(const std::wstring& src, const std::wstring& dst) {
        try {
            fs::path srcPath(src);
            fs::path dstPath(dst);

            // No, if the target already exists and is a file, delete it first
            if (fs::exists(dstPath) && fs::is_regular_file(dstPath)) {
                fs::remove(dstPath);
            }

            // Move/Rename
            fs::rename(srcPath, dstPath);
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    bool GetFileDirectory(const std::wstring& src, std::wstring& dst)
    {
        size_t pos = src.find_last_of(L"\\");
        if (pos != std::wstring::npos) {
            dst = src.substr(0, pos);
            return true;
        }
        return false;
    }

    bool GetFileName(const std::wstring& src, std::wstring& dst)
    {
        try {
            std::filesystem::path path(src);
            // Get FileName
            std::filesystem::path filename = path.filename();
            // Remove the extension
            std::filesystem::path stem = filename.stem();

            dst = stem.wstring();
            return !dst.empty();
        }
        catch (const std::filesystem::filesystem_error&) {
            dst.clear();
            return false;
        }
    }

    bool CreateDirectory(const std::wstring& path) {
        fs::path p(path);
        std::error_code error;
        fs::create_directories(p, error);
        if (!error) {
            return true;
        }

        std::error_code statusError;
        return fs::is_directory(p, statusError) && !statusError;
    }

    bool Exists(const std::wstring& path) {
        return fs::exists(fs::path(path));
    }

    std::vector<std::string> GetFilesInDirectory(const std::wstring& path) {
        fs::path dir(path);
        std::vector<std::string> files;
        if (!fs::is_directory(dir)) {
            return files;
        }
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry)) {
                files.push_back(entry.path().filename().string());
            }
        }
        return files;
    }

} // namespace IO