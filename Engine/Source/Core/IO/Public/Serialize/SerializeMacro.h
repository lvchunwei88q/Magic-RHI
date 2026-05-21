#pragma once

/*
* 这个序列化宏定义了使用IO模块系统来进行加载与创建序列化数据，这也要求必须传入支持cereal序列化的类
* #include <AbsolutePath.h>
* #include <FileManager.h>
* #include <fstream>
* #include <ILog.h>
*/
#define FILE_SERIALIZATION_LOADING(object,path,name)													\
const std::wstring object##_Executable = IO::AbsolutePath::Get().GetExecutableDirectory();				\
const std::wstring object##_context_dir = object##_Executable + L"\\" + path;							\
const std::wstring object##_context_path = object##_Executable + L"\\" + path + name;					\
if (IO::FileManager::Exists(object##_context_path)) {													\
	std::ifstream file(object##_context_path);															\
    try {																								\
		cereal::BinaryInputArchive archive(file);														\
		archive(object);																				\
	}																									\
	catch (const std::exception& e) {																	\
		LOG_WARNING("Serialized Read: " + std::string(e.what()));										\
	}																									\
}																										\
else {																									\
	std::string BinaryStr;																				\
	{																									\
		std::ostringstream oss;																			\
		cereal::BinaryOutputArchive archive(oss);														\
		archive(object);																				\
		BinaryStr = oss.str();																			\
	}																									\
																										\
	IO::FileManager::MakeDirectory(object##_context_dir);												\
	IO::FileManager::MakeFile(object##_context_path);													\
	IO::FileManager::WriteAllText(object##_context_path, BinaryStr);									\
}																										

#define FILE_SERIALIZATION_SAVE(object,path,name)														\
{																										\
	const std::wstring object##_Executable = IO::AbsolutePath::Get().GetExecutableDirectory();			\
	const std::wstring object##_context_dir = object##_Executable + L"\\" + path;						\
	const std::wstring object##_context_path = object##_Executable + L"\\" + path + name;				\
	std::string BinaryStr;																				\
	{																									\
		std::ostringstream oss;																			\
		cereal::BinaryOutputArchive archive(oss);														\
		archive(object);																				\
		BinaryStr = oss.str();																			\
	}																									\
																										\
	IO::FileManager::MakeDirectory(object##_context_dir);												\
	IO::FileManager::MakeFile(object##_context_path);													\
	IO::FileManager::WriteAllText(object##_context_path, BinaryStr);									\
}
