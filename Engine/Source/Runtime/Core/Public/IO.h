#pragma once

#include "Common/Core_API.h"
#include "Common/Compiler.h"
#include "Tools/Singleton.h"
#include <string>
#include <vector>

#define CACHE L"Cache\\"
#define CONFIG L"Config\\"

DISABLE_DLL_WARNINGS_PUSH;

namespace IO {
	// 读取整个文件（文本模式，返回字符串）
	CORE_API std::string ReadAllText(const std::wstring& path);

	// 读取整个文件（二进制模式，返回字节向量）
	CORE_API std::vector<char> ReadAllBytes(const std::wstring& path);

	// 写入字符串到文件（覆盖）
	CORE_API void WriteAllText(const std::wstring& path, const std::string& content);

	// 写入字节到文件（覆盖）
	CORE_API void WriteAllBytes(const std::wstring& path, const std::vector<char>& data);

	// 追加字符串到文件末尾
	CORE_API void AppendText(const std::wstring& path, const std::string& content);

	// 追加字节到文件末尾
	CORE_API void AppendBytes(const std::wstring& path, const std::vector<char>& data);

	// 删除文件
	CORE_API bool DeleteToFile(const std::wstring& path);

	// 删除文件夹
	CORE_API bool DeleteToDirectory(const std::wstring& path);

	// 创建空文件（如果文件已存在则更新其时间戳）
	CORE_API bool MakeFile(const std::wstring& path);

	// 移动/重命名文件或目录
	CORE_API bool MoveFile(const std::wstring& src, const std::wstring& dst);

	// 获取文件的目录
	CORE_API bool GetFileDirectory(const std::wstring& src, std::wstring& dst);

	// 创建目录（包括父目录）
	CORE_API bool MakeDirectory(const std::wstring& path);

	// 检查文件或目录是否存在
	CORE_API bool Exists(const std::wstring& path);

	// 扫描指定目录下的所有文件（不含子目录），返回文件名列表
	CORE_API std::vector<std::string> GetFilesInDirectory(const std::wstring& path);

	// 窄字符转宽字符 (UTF-8 to UTF-16)
	CORE_API std::wstring ToWideString(const std::string& narrow);
	// 宽字符转窄字符 (UTF-16 to UTF-8)
	CORE_API std::string ToNarrowString(const std::wstring& wide);

	class CORE_API AbsolutePath : public Singleton<AbsolutePath> // 绝对路径
	{
	public:
		AbsolutePath() = default;

		/**
		 * 获取可执行文件所在目录
		 * @return 可执行文件的完整目录路径（不包含文件名）
		 */
		std::wstring GetExecutableDirectory();

		/**
		 * 获取当前工作目录
		 * @return 当前工作目录路径
		 */
		std::wstring GetCurrentWorkingDirectory();

		/**
		 * 获取可执行文件的完整路径（包含文件名）
		 * @return 可执行文件的完整路径
		 */
		std::wstring GetExecutablePath();

		/**
		* 获取内容路径
		* @return 内容路径完整路径
		*/
		std::wstring GetContentPath();

		/**
		* 获取脚本路径
		* @return 脚本路径完整路径
		*/
		std::wstring GetScriptPath();

		/**
		* 清除缓存
		*/
		void ClearCache();
	private:
		// 缓存路径结果，避免重复计算
		std::wstring ExecutableDirectory;
		std::wstring CurrentWorkingDirectory;
		std::wstring ExecutablePath;
		std::wstring ContentPath;
		std::wstring ScriptPath;
	};
}

DISABLE_DLL_WARNINGS_POP;