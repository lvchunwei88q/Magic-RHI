#pragma once
using HRESULT = long;
using UINT = unsigned int;

#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#include <stdexcept>

namespace RHI
{
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

    inline void ThrowIfFailed(const char* message)
    {
        throw std::runtime_error(message);
    }
}