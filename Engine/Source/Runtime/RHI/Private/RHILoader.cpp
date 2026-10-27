#include "RHILoader.h"
#include "Tools/EnumClassFlags.h"
#include <RHI.modules.generated.h>

#include <type_traits>

using namespace Core;
namespace RHI
{
    AUTO_REGISTER(RHILoader);
    
    IRHILoader* IRHIModule::GetRHILoader(){
        return &RHILoader::Get();
    }

    RHILoader::~RHILoader(){}

    namespace {
        bool IsSingleRHIType(RHIType type)
        {
            // if the type is None, it's not a valid single RHI type
            if (type == RHIType::None) return false;
            // Check if the type is Unknown, which is also not a valid single RHI type
            if (type == RHIType::Unknown) return false;

            // Convert the RHIType to its underlying integer representation
            using UnderlyingType = std::underlying_type_t<RHIType>;
            UnderlyingType val = static_cast<UnderlyingType>(type);

            // Check if the value is a power of two, which indicates that only one bit is set
            return (val & (val - 1)) == 0;
        }
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

        if(!IsSingleRHIType(type)){
            ErrorCapture::Capture("Invalid RHI type: " + std::string(RHITypeToString(type)) + ". Only single RHI types are allowed.");
            return false;
        }

        // Determine the DLL name based on the RHI type
        if (EnumHasAnyFlags(type, RHIType::D3D12)) {
            dllName = RHI_D3D12_DLL_FILE;
        }else if (EnumHasAnyFlags(type, RHIType::VulKan)) {
            dllName = RHI_VULKAN_DLL_FILE;
        }// ... add more RHI types here as needed
        
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
        m_CreateShaderCompilerBackend = reinterpret_cast<PFN_CreateShaderCompilerBackend>(GetProcAddress(m_hModule, "CreateShaderCompilerBackend"));
        m_GetRHIType = reinterpret_cast<PFN_GetRHIType>(GetProcAddress(m_hModule, "GetRHIType"));
        
        if (!m_CreateDevice || !m_CreateSwapChain || !m_CreateShaderCompilerBackend || !m_GetRHIType) {
            Unload();
            ErrorCapture::Capture("Failed to get RHI function pointers");
            return false;
        }
        
        // verify type match
        if (m_GetRHIType() != type) {
            Unload();
            ErrorCapture::Capture("RHI type mismatch: " + std::string(RHITypeToString(type)));
            return false;
        }
        
        InfoCapture::Capture("RHI type match: " + std::string(RHITypeToString(type)));
        
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
        m_CreateShaderCompilerBackend = nullptr;
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

    std::unique_ptr<ShaderCompilerBackend> RHILoader::CreateShaderCompilerBackend()
    {
        if (m_CreateShaderCompilerBackend) {
            return m_CreateShaderCompilerBackend();
        }
        return nullptr;
    }

} // namespace RHI