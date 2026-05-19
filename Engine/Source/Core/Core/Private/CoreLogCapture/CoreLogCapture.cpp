#include "CoreLogCapture/CoreLogCapture.h"
#include <Windows.h> // OutputDebugStringA

namespace Core {

#ifdef _DEBUG
#define LOG_OUTPUT(msg) OutputDebugStringA(msg)
#else
#define LOG_OUTPUT(msg) ((void)0)
#endif

#define TEMPPLATE_CAPTURE_IMPL(Target)                                                  \
    Target##Capture::FuncType Target##Capture::GetCaptureFunction;                      \
    void Target##Capture::RegisterCaptureFunction(FuncType func) {                      \
        GetCaptureFunction = func;                                                      \
    }                                                                                   \
    void Target##Capture::Capture(const std::string& Message) {                         \
        if (GetCaptureFunction) {                                                       \
            GetCaptureFunction(Message);                                                \
        }                                                                               \
        else {                                                                          \
            LOG_OUTPUT("Error CaptureFunction is nullptr! CaptureError:");              \
            LOG_OUTPUT(Message.c_str());                                                \
            LOG_OUTPUT("\n");                                                           \
        }                                                                               \
    }

    TEMPPLATE_CAPTURE_IMPL(Error)
    TEMPPLATE_CAPTURE_IMPL(Warning)
    TEMPPLATE_CAPTURE_IMPL(Info)
}