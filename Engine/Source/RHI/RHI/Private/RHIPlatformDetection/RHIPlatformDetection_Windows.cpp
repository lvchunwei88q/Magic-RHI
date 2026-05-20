#include "Common/RHIPlatformDetection.h"
#include <windows.h>

static bool IsD3D11Available()
{
    static bool available = ([]() {
        HMODULE dll = LoadLibraryA("d3d11.dll");
        if (dll) FreeLibrary(dll);
        return dll != nullptr;
    })();
    return available;
}
static bool IsD3D12Available()
{
    static bool available = ([]() {
        HMODULE dll = LoadLibraryA("d3d12.dll");
        if (dll) FreeLibrary(dll);
        return dll != nullptr;
    })();
    return available;
}

namespace RHI
{
    RHIType GetBestAvailableRHI()
    {
        // Windows 优先级：D3D12 > D3D11 -> Unknown
        if (IsD3D12Available()) return RHIType::DirectX12;
        if (IsD3D11Available()) return RHIType::DirectX11;
        
        return RHIType::Unknown;
    }
}