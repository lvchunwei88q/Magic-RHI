#pragma once
using HRESULT = long;
using UINT = unsigned int;

#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#include <stdexcept>
#include <CoreLogCapture/CoreLogCapture.h>

namespace RHI
{
    enum class CoreLogType
    {
        Error,
        Warning,
        Info,
    };

    inline std::string HrToString(HRESULT hr)
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
        return std::string(s_str);
    }

    class HrException : public std::runtime_error
    {
    public:
        HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
        HRESULT Error() const { return m_hr; }
    private:
        const HRESULT m_hr;
    };

    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw HrException(hr);
        }
    }

    inline void CoreLog(const std::string& message, CoreLogType type = CoreLogType::Info){
        switch (type)
        {
        case CoreLogType::Error:
            Core::ErrorCapture::Capture(message);
            break;
        case CoreLogType::Warning:
            Core::WarningCapture::Capture(message);
            break;
        case CoreLogType::Info:
            Core::InfoCapture::Capture(message);
            break;
        default:
            break;
        }
    }

    template<typename T>
    inline void ThrowErrorMessage(T&& message)
    {
        throw std::runtime_error(std::forward<T>(message));
    }
}