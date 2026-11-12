#pragma once

#include "Common/CORE_API.h"
#include "Common/Checker.h"
#include "Utilities/Singleton.h"
#include "Utilities/BinaryRW.h"
#include <string>
#include <vector>

#define CACHE L"Cache\\"
#define CONFIG L"Config\\"

// Get rid of Windows macro clutter
#undef CreateFile
#undef CreateDirectory
#undef MoveFile


DISABLE_DLL_WARNINGS_PUSH;

/*
* Note that the 'bool' here doesn't indicate whether an operation succeeded or failed;
* in some cases, it might mean something different. For details, check the STL source code.
*/

namespace IO {
	// Read text to file
	CORE_API std::string ReadAllText(const std::wstring& path);
	// Read bytes to file
	CORE_API std::vector<char> ReadAllCharBytes(const std::wstring& path);
	CORE_API std::vector<uint8_t> ReadAllUINT8Bytes(const std::wstring& path);
	CORE_API BinaryReader ReadAllBinaryReaderBytes(const std::wstring& path);
	// Write string to file
	CORE_API void WriteAllText(const std::wstring& path, const std::string& content);
	// Write byte to file
	CORE_API void WriteAllBytes(const std::wstring& path, const std::vector<char>& data);
	CORE_API void WriteAllBytes(const std::wstring& path, const std::vector<uint8_t>& data);
	CORE_API void WriteAllBytes(const std::wstring& path, const BinaryWriter& data);
	CORE_API void WriteAllBytes(const std::wstring& path, const uint8_t* data, size_t size);
	CORE_API void WriteAllBytes(const std::wstring& path, const char* data, size_t size);
	// Append a string to the end of the file
	CORE_API void AppendText(const std::wstring& path, const std::string& content);
	// Append bytes to the end of the file
	CORE_API void AppendBytes(const std::wstring& path, const std::vector<char>& data);
	CORE_API void AppendBytes(const std::wstring& path, const std::vector<uint8_t>& data);
	CORE_API void AppendBytes(const std::wstring& path, const BinaryWriter& data);
	CORE_API void AppendBytes(const std::wstring& path, const uint8_t* data, size_t size);
	CORE_API void AppendBytes(const std::wstring& path, const char* data, size_t size);
	// deleta file
	CORE_API bool DeleteToFile(const std::wstring& path);
	// deleta directory
	CORE_API bool DeleteToDirectory(const std::wstring& path);
	// Create an empty file (or update its timestamp if the file already exists)
	CORE_API bool CreateFile(const std::wstring& path);
	// Move/Rename Files or Folders
	CORE_API bool MoveFile(const std::wstring& src, const std::wstring& dst);
	// Get the file directory
	CORE_API bool GetFileDirectory(const std::wstring& src, std::wstring& dst);
	// Get the file name
	CORE_API bool GetFileName(const std::wstring& src, std::wstring& dst);
	// Create directory (including parent directories)
	CORE_API bool CreateDirectory(const std::wstring& path);
	// Check if a file or directory exists
	CORE_API bool Exists(const std::wstring& path);
	// Scan all files in the specified directory (excluding subdirectories) and return a list of file names
	CORE_API std::vector<std::string> GetFilesInDirectory(const std::wstring& path);
	// Convert narrow characters to wide characters (UTF-8 to UTF-16)
	CORE_API std::wstring ToWideString(const std::string& narrow);
	// Convert wide characters to narrow characters (UTF-16 to UTF-8)
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

	class CORE_API AbsolutePath : public Singleton<AbsolutePath> // Absolute Path
	{
	public:
		AbsolutePath() = default;

		// Get the directory where the executable file is located
		std::wstring GetExecutableDirectory();
		// Get the full path of the executable file
		std::wstring GetExecutablePath();

		// Get current working directory
		WorkingDirectory GetCurrentWorkingDirectory();
		// Get content directory
		ContentDirectory GetContentDirectory();
		// Get script directory
		ScriptDirectory GetScriptDirectory();

		// Clear cache
		void ClearCache();
	private:
		// Cache the path results to avoid recalculating
		std::wstring ExecutableDirectory;
		std::wstring CurrentWorkingDirectory;
		std::wstring ExecutablePath;
		std::wstring ContentPath;
		std::wstring ScriptPath;
	};
}

DISABLE_DLL_WARNINGS_POP;