#include "RHILoader.h"
#include "RHIConfig.h"

namespace RHI
{
    IRHILoader* GetLoader(){
        return &RHILoader::Get();
    }

    RHILoader::~RHILoader()
    {
        Unload();
    }

    bool RHILoader::Load(RHIType type)
    {   
        const char* dllName = "";

        switch (type)
        {
        case RHIType::DirectX12:
            dllName = RHI_D3D12_DLL_FILE;
            break;
        case RHIType::DirectX11:
            dllName = RHI_D3D11_DLL_FILE;
            break;
        default:
            return false;       
        }
        
        // load dll
        m_hModule = LoadLibraryA(dllName);
        if (!m_hModule) {
            DWORD error = GetLastError();
            
            return false;
        }
        
        // get function pointers
        m_CreateDevice = reinterpret_cast<PFN_CreateDevice>(GetProcAddress(m_hModule, "CreateDevice"));
        m_CreateSwapChain = reinterpret_cast<PFN_CreateSwapChain>(GetProcAddress(m_hModule, "CreateSwapChain"));
        m_GetRHIType = reinterpret_cast<PFN_GetRHIType>(GetProcAddress(m_hModule, "GetRHIType"));
        
        if (!m_CreateDevice || !m_CreateSwapChain || !m_GetRHIType) {
            Unload();
            return false;
        }
        
        // verify type match
        if (m_GetRHIType() != type) {
            Unload();
            return false;
        }
        
        m_loadedType = type;
        return true;
    }

    void RHILoader::Unload()
    {
        if (m_hModule) {
            FreeLibrary(m_hModule);
            m_hModule = nullptr;
        }
        
        m_CreateDevice = nullptr;
        m_CreateSwapChain = nullptr;
        m_GetRHIType = nullptr;
    }

    std::unique_ptr<Device> RHILoader::CreateDevice()
    {
        if (m_CreateDevice) {
            return m_CreateDevice();
        }
        return nullptr;
    }

    std::unique_ptr<SwapChain> RHILoader::CreateSwapChain(Device* device, const SwapChainDesc& desc)
    {
        if (m_CreateSwapChain) {
            return m_CreateSwapChain(device, desc);
        }
        return nullptr;
    }

} // namespace RHI