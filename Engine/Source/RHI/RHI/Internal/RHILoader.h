#pragma once

#include <memory>
#include <windows.h>
#include <Tools/Singleton.h>
#include "RHIDynamicLoader.h"
#include "RHI.h"
#include "IRHILoader.h"

namespace RHI
{
    class RHILoader : public Singleton<RHILoader> , public IRHILoader
    {
    public:
        RHILoader() = default;
        ~RHILoader();

        bool Load(RHIType type);
        void Unload();
        
        std::unique_ptr<Device> CreateDevice();
        std::unique_ptr<SwapChain> CreateSwapChain(Device* device, const SwapChainDesc& desc);
        
        bool IsLoaded() const { return m_hModule != nullptr; }
        RHIType GetLoadedRHIType() const { return m_loadedType; }
        
    private:
        
        HMODULE m_hModule = nullptr;
        PFN_CreateDevice m_CreateDevice = nullptr;
        PFN_CreateSwapChain m_CreateSwapChain = nullptr;
        PFN_GetRHIType m_GetRHIType = nullptr;
        RHIType m_loadedType = RHIType::DirectX11;
    };
} // namespace RHI