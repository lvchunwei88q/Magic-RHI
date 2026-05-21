#pragma once
#include "Common/IO_API.h"

#include <string>
#include <vector>

namespace IO {
    class IO_API FileManager {
    public:
        // 禁止实例化
        FileManager() = delete;

        // 读取整个文件（文本模式，返回字符串）
        static std::string ReadAllText(const std::wstring& path);

        // 读取整个文件（二进制模式，返回字节向量）
        static std::vector<char> ReadAllBytes(const std::wstring& path);

        // 写入字符串到文件（覆盖）
        static void WriteAllText(const std::wstring& path, const std::string& content);

        // 写入字节到文件（覆盖）
        static void WriteAllBytes(const std::wstring& path, const std::vector<char>& data);

        // 追加字符串到文件末尾
        static void AppendText(const std::wstring& path, const std::string& content);

        // 追加字节到文件末尾
        static void AppendBytes(const std::wstring& path, const std::vector<char>& data);

        // 删除文件
        static bool DeleteToFile(const std::wstring& path);

        // 删除文件夹
        static bool DeleteToDirectory(const std::wstring& path);

        // 创建空文件（如果文件已存在则更新其时间戳）
        static bool MakeFile(const std::wstring& path);

        // 移动/重命名文件或目录
        static bool MoveFile(const std::wstring& src, const std::wstring& dst);

        // 获取文件的目录
        static bool GetFileDirectory(const std::wstring& src, std::wstring& dst);

        // 创建目录（包括父目录）
        static bool MakeDirectory(const std::wstring& path);

        // 检查文件或目录是否存在
        static bool Exists(const std::wstring& path);

        // 扫描指定目录下的所有文件（不含子目录），返回文件名列表
        static std::vector<std::string> GetFilesInDirectory(const std::wstring& path);
    };
} // namespace IO