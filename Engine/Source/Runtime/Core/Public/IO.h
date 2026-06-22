#pragma once

#include "Common/Core_API.h"
#include "Common/Check.h"
#include "Tools/Singleton.h"
#include "Tools/Buffer.h"
#include <string>
#include <vector>

#define CACHE L"Cache\\"
#define CONFIG L"Config\\"

#ifdef MoveFile
#undef MoveFile 
#endif

DISABLE_DLL_WARNINGS_PUSH;

/*
* 注意这里的bool不代表操作成功或失败，具体请参考STL源码
*/

namespace IO {
	// 读取整个文件
	CORE_API std::string ReadAllText(const std::wstring& path);
	CORE_API std::vector<char> ReadAllBytes(const std::wstring& path);
	CORE_API std::vector<uint8_t> ReadAllU8Bytes(const std::wstring& path);
	// 写入字符串到文件
	CORE_API void WriteAllText(const std::wstring& path, const std::string& content);
	// 写入字节到文件
	CORE_API void WriteAllBytes(const std::wstring& path, const BinaryWrite& data);
	// 追加字符串到文件末尾
	CORE_API void AppendText(const std::wstring& path, const std::string& content);
	// 追加字节到文件末尾
	CORE_API void AppendBytes(const std::wstring& path, const BinaryWrite& data);
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
	// 获取文件的名称
	CORE_API bool GetFileName(const std::wstring& src, std::wstring& dst);
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

	struct BaseDirectory {
		explicit BaseDirectory(const std::wstring& Root) : Root(Root) {};
		BaseDirectory() = delete;

		const std::wstring GetRoot() { return Root; }
	private:
		std::wstring Root;
	};
	// WorkingDirectory == Project Directory
	struct WorkingDirectory : public BaseDirectory
	{
		explicit WorkingDirectory(const std::wstring& Root) : BaseDirectory(Root) {};
		WorkingDirectory() = delete;

		std::wstring GetContent()	{ return GetRoot() + L"\\Content\\";	};
		std::wstring GetConfig()	{ return GetRoot() + L"\\Config\\";		};
		std::wstring GetCache()		{ return GetRoot() + L"\\Cache\\";		};
	};

	struct ContentDirectory : public BaseDirectory
	{
		explicit ContentDirectory(const std::wstring& Root) : BaseDirectory(Root) {};
		ContentDirectory() = delete;

		std::wstring GetEditor() { return GetRoot() + L"\\Editor\\"; };
		std::wstring GetConfig() { return GetRoot() + L"\\Config\\"; };
		std::wstring GetLogo()   { return GetRoot() + L"\\Logo\\";   };
		std::wstring GetWindow() { return GetRoot() + L"\\Window\\"; };
	};

	struct ScriptDirectory : public BaseDirectory
	{
		explicit ScriptDirectory(const std::wstring& Root) : BaseDirectory(Root) {};
		ScriptDirectory() = delete;
	};

	class CORE_API AbsolutePath : public Singleton<AbsolutePath> // 绝对路径
	{
	public:
		AbsolutePath() = default;

		// 获取可执行文件所在目录
		std::wstring GetExecutableDirectory();
		// 获取可执行文件的完整路径
		std::wstring GetExecutablePath();

		// 获取当前工作目录
		WorkingDirectory GetCurrentWorkingDirectory();
		// 获取内容路径
		ContentDirectory GetContentDirectory();
		// 获取脚本路径
		ScriptDirectory GetScriptDirectory();

		// 清除缓存
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