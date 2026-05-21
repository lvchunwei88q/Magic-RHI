#pragma once
#include "Common/IO_API.h"
#include <Tools/Singleton.h>
#include <string>

#define CACHE L"Cache\\"
#define CONFIG L"Config\\"

namespace IO {
	class IO_API AbsolutePath : public Singleton<AbsolutePath> // 绝对路径子系统
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
	};
};