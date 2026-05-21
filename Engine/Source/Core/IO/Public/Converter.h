#pragma once
#include "Common/IO_API.h"
#include <string>

namespace IO {
	class IO_API Converter
	{
	public:
		Converter() = default;

		// 窄字符转宽字符 (UTF-8 to UTF-16)
		static std::wstring ToWideString(const std::string& narrow);

		// 宽字符转窄字符 (UTF-16 to UTF-8)
		static std::string ToNarrowString(const std::wstring& wide);
	};
};