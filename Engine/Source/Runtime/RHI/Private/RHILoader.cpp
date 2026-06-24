#include "RHILoader.h"
#include <CoreLogCapture/CoreLogCapture.h>
#include <RHI.modules.generated.h>

using namespace Core;
namespace RHI
{
    AUTO_REGISTER(RHILoader);
    
    IRHILoader* GetLoader(){
        return &RHILoader::Get();
    }

    RHILoader::~RHILoader()
    {
    }

    bool RHILoader::Init()
    {
        return true;
    }

    void RHILoader::Uninstall()
    {
        Unload();
    }

    bool RHILoader::Load(RHIType type)
    {   
        const char* dllName = "";

        switch (type)
        {
        case RHIType::D3D12:
            dllName = RHI_D3D12_DLL_FILE;
            break;
        case RHIType::D3D11:
            dllName = RHI_D3D11_DLL_FILE;
            break;
        default:
            return false;       
        }
        
        // load dll
        InfoCapture::Capture("Loading RHI DLL: " + std::string(dllName));
        m_hModule = LoadLibraryA(dllName);
        if (!m_hModule) {
            DWORD error = GetLastError();
            ErrorCapture::Capture("Failed to load RHI DLL: " + std::string(dllName) 
                      + " with error code: " + std::to_string(error));
            return false;
        }
        
        // get function pointers
        m_CreateDevice = reinterpret_cast<PFN_CreateDevice>(GetProcAddress(m_hModule, "CreateDevice"));
        m_CreateSwapChain = reinterpret_cast<PFN_CreateSwapChain>(GetProcAddress(m_hModule, "CreateSwapChain"));
        m_GetRHIType = reinterpret_cast<PFN_GetRHIType>(GetProcAddress(m_hModule, "GetRHIType"));
        
        if (!m_CreateDevice || !m_CreateSwapChain || !m_GetRHIType) {
            Unload();
            ErrorCapture::Capture("Failed to get RHI function pointers");
            return false;
        }
        
        // verify type match
        if (m_GetRHIType() != type) {
            Unload();
            ErrorCapture::Capture("RHI type mismatch: " + std::string(ToString(type)));
            return false;
        }
        
        InfoCapture::Capture("RHI type match: " + std::string(ToString(type)));
        
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

    std::unique_ptr<SwapChain> RHILoader::CreateSwapChain()
    {
        if (m_CreateSwapChain) {
            return m_CreateSwapChain();
        }
        return nullptr;
    }

} // namespace RHI