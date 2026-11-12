#pragma once
#include "Common/CORE_API.h"

////////////////////////////// Engine Base Include
#include "Common/Checker.h"
#include "Common/Math.h"
//////////////////////////////

////////////////////// from STL
#include <string>

/*
* Error catching allows you to not rely on the system logging module,
* but in the end, it will use the logging system to record error information.
* So its core object-oriented focus is the Core module, because the Core module can't reference the logging module.
*/
#define TEMPPLATE_CAPTURE(Target)                               \
        class CORE_API Target##Capture {                            \
        public:                                                     \
            using FuncType = void(*)(const std::string& Message);   \
        private:                                                    \
            static FuncType GetCaptureFunction;                     \
        public:                                                     \
            static void RegisterCaptureFunction(FuncType func);     \
            static void Capture(const std::string& Message);        \
            void operator()(const std::string& Message) const {     \
                Capture(Message);                                   \
            }                                                       \
        };

namespace Core
{
    TEMPPLATE_CAPTURE(Error);
    TEMPPLATE_CAPTURE(Warning);
    TEMPPLATE_CAPTURE(Info);

	class CORE_API Core
	{
	public:
		Core();
		~Core();
		static const char* GetVersion();
	private:

	};
}