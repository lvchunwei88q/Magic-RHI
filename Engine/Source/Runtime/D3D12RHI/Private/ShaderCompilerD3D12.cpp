#include "RHIResourceD3D12.h"
#include <Common/RHIDefinitions.h>
#include <Common/Check.h>
#include "RHID3D12.h"

#include <IO.h>


namespace RHI
{
    namespace
    {
        // Convert the D3D_SHADER_MODEL enum to a custom enum
        ShaderModelVersion ConvertD3DShaderModel(D3D_SHADER_MODEL model)
        {
            switch (model)
            {
            case D3D_SHADER_MODEL_5_1: return ShaderModelVersion::SM_5_1;
            case D3D_SHADER_MODEL_6_0: return ShaderModelVersion::SM_6_0;
            case D3D_SHADER_MODEL_6_1: return ShaderModelVersion::SM_6_1;
            case D3D_SHADER_MODEL_6_2: return ShaderModelVersion::SM_6_2;
            case D3D_SHADER_MODEL_6_3: return ShaderModelVersion::SM_6_3;
            case D3D_SHADER_MODEL_6_4: return ShaderModelVersion::SM_6_4;
            case D3D_SHADER_MODEL_6_5: return ShaderModelVersion::SM_6_5;
            case D3D_SHADER_MODEL_6_6: return ShaderModelVersion::SM_6_6;
            case D3D_SHADER_MODEL_6_7: return ShaderModelVersion::SM_6_7;
            case D3D_SHADER_MODEL_6_8: return ShaderModelVersion::SM_6_8;
            case D3D_SHADER_MODEL_6_9: return ShaderModelVersion::SM_6_9;
            default: return ShaderModelVersion::Unknown;
            }
        }

        // Get the highest Shader Model supported by the device
        ShaderModelVersion GetHighestSupportedShaderModel(ID3D12Device* device)
        {
            if (!device)
                return ShaderModelVersion::Unknown;
            
            // Test all SM versions in descending order to find the highest supported one
            std::vector<D3D_SHADER_MODEL> testVersions = {
                D3D_SHADER_MODEL_6_9,
                D3D_SHADER_MODEL_6_8,
                D3D_SHADER_MODEL_6_7,
                D3D_SHADER_MODEL_6_6,
                D3D_SHADER_MODEL_6_5,
                D3D_SHADER_MODEL_6_4,
                D3D_SHADER_MODEL_6_3,
                D3D_SHADER_MODEL_6_2,
                D3D_SHADER_MODEL_6_1,
                D3D_SHADER_MODEL_6_0,
                D3D_SHADER_MODEL_5_1,
            };
            
            D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
            
            for (D3D_SHADER_MODEL version : testVersions)
            {
                shaderModel.HighestShaderModel = version;
                
                if (SUCCEEDED(device->CheckFeatureSupport(
                    D3D12_FEATURE_SHADER_MODEL, 
                    &shaderModel, 
                    sizeof(shaderModel))))
                {
                    // Return the highest supported version
                    return ConvertD3DShaderModel(shaderModel.HighestShaderModel);
                }
            }
            
            return ShaderModelVersion::Unknown;
        }

        template<typename ShaderType>
        std::unique_ptr<ShaderType> CompileShaderInternal(const CreateShaderDesc& desc)
        {
            if(desc.shaderType != CreateShaderDesc::ShaderType::HLSL){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("Shader type is not HLSL");
#endif 
                return nullptr;
            }
            return std::make_unique<ShaderType>(desc.GetHLSLByteCode());
        }
    }

    bool CreateShaderD3D12::Initialize(Device* device)
    {
        m_pRHI = SafeCast<DeviceD3D12>(device);
        m_Initialization = CoreDeviceInitialization::Initialize;
        return true;
    }

    void CreateShaderD3D12::Shutdown()
    {
        m_Initialization = CoreDeviceInitialization::Shutdown;
        m_pRHI = nullptr;
    }

    bool CreateShaderD3D12::IsValid() const
    {
        return m_Initialization == CoreDeviceInitialization::Initialize;
    }

    std::unique_ptr<RHIVertexShader> CreateShaderD3D12::CreateVertexShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<VertexShaderD3D12>(desc);
    }

    std::unique_ptr<RHIPixelShader> CreateShaderD3D12::CreatePixelShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<PixelShaderD3D12>(desc);
    }

    std::unique_ptr<RHIGeometryShader> CreateShaderD3D12::CreateGeometryShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderD3D12>(desc);
    }

    std::unique_ptr<RHIHullShader> CreateShaderD3D12::CreateHullShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<HullShaderD3D12>(desc);
    }

    std::unique_ptr<RHIDomainShader> CreateShaderD3D12::CreateDomainShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<DomainShaderD3D12>(desc);
    }

    std::unique_ptr<RHIComputeShader> CreateShaderD3D12::CreateComputeShader(const CreateShaderDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderD3D12>(desc);
    }
    
    ShaderModelVersion CreateShaderD3D12::GetShaderModelVersion() const
    {
        return GetHighestSupportedShaderModel(m_pRHI->GetDevice());
    }
}
