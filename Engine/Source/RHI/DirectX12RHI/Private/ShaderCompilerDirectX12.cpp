#include "RHIResourceDirectX12.h"
#include "RHIDirectX12.h"
#include "DirectXHelper.h"

#include <Converter.h>
#include <FileManager.h>

namespace RHI
{
    namespace
    {
        // 将 D3D_SHADER_MODEL 枚举转换为自定义枚举
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
            default: return ShaderModelVersion::Unknown;
            }
        }

        // 获取设备支持的最高 Shader Model
        ShaderModelVersion GetHighestSupportedShaderModel(ID3D12Device* device)
        {
            if (!device)
                return ShaderModelVersion::Unknown;
            
            // 按从高到低的顺序测试所有 SM 版本
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
                    // 返回检测到的最高版本
                    return ConvertD3DShaderModel(shaderModel.HighestShaderModel);
                }
            }
            
            return ShaderModelVersion::Unknown;
        }

        std::string ReadFileToString(const std::string& filePath)
        {
            // 检查文件是否存在
            std::wstring filePathW = IO::Converter::ToWideString(filePath);
            if (!IO::FileManager::Exists(filePathW))
            {
                ThrowErrorMessage("Shader file not found: " + filePath);
                return "";
            }

            std::string content = IO::FileManager::ReadAllText(filePathW);
            return content;
        }

        std::string LoadShaderSource(const ShaderCompileDesc& desc, std::string& outBasePath)
        {
            if (desc.SourceCode)
            {
                return desc.SourceCode;
            }
            else if (desc.FilePath)
            {
                std::string filePath = desc.FilePath;
                size_t lastSlash = filePath.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                {
                    outBasePath = filePath.substr(0, lastSlash);
                }
                
                return ReadFileToString(filePath);
            }
            else
            {
                ThrowErrorMessage("No shader source or file path provided");
                return "";
            }
        }

        template<typename ShaderType>
        std::shared_ptr<ShaderType> CompileShaderInternal(RHIDirectX12* device, const ShaderCompileDesc& desc, const char* defaultProfile)
        {
            std::string basePath;
            std::string source = LoadShaderSource(desc, basePath);
            if (source.empty())
                return nullptr;

            std::string profile = desc.Profile ? desc.Profile : defaultProfile;
            std::string entryPoint = desc.EntryPoint ? desc.EntryPoint : "main";

            std::vector<uint8_t> bytecode;
            if (!device->CompileShaderToBytecode(source, entryPoint, profile, desc.EnableDebugInfo, bytecode))
                return nullptr;

            return std::make_shared<ShaderType>(bytecode);
        }
    }

    std::string RHIDirectX12::GetShaderTarget(const char* prefix) const
    {
        ShaderModelVersion highestSM = GetHighestSupportedShaderModel(m_pDevice.Get());
        
        if (highestSM >= ShaderModelVersion::SM_6_0)
        {
            return std::string(prefix) + "_" + ShaderModelToString(highestSM);
        }
        else if (highestSM >= ShaderModelVersion::SM_5_1)
        {
            return std::string(prefix) + "_5_1";
        }
        else
        {
            return std::string(prefix) + "_5_0";
        }
    }

    bool RHIDirectX12::CompileShaderToBytecode(const std::string& source, const std::string& entryPoint, 
                                const std::string& profile, bool enableDebug,
                                std::vector<uint8_t>& outBytecode)
    {   
        if (!compiler || !utils)
        {
            ThrowErrorMessage("DXC not initialized");
            return false;
        }
        
        // 转换字符串为 UTF-16
        std::wstring sourceW = IO::Converter::ToWideString(source);
        std::wstring entryPointW = IO::Converter::ToWideString(entryPoint);
        std::wstring profileW = IO::Converter::ToWideString(profile);
        
        // 创建源码 Blob
        ComPtr<IDxcBlobEncoding> sourceBlob;
        ThrowIfFailed(utils->CreateBlob(
            sourceW.c_str(),
            static_cast<UINT32>(sourceW.size() * sizeof(wchar_t)),
            DXC_CP_WIDE,
            &sourceBlob
        ));
        
        // 准备参数
        std::vector<LPCWSTR> args;
        args.push_back(L"-T"); args.push_back(profileW.c_str());
        args.push_back(L"-E"); args.push_back(entryPointW.c_str());
        
        if (enableDebug)
        {
            args.push_back(L"-Zi");
            args.push_back(L"-Od");
        }
        
        // 开启严格模式
        args.push_back(L"-Ges");
        args.push_back(L"-WX");
        
        // 编译
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();
        sourceBuffer.Encoding = DXC_CP_WIDE;
        
        ComPtr<IDxcResult> result;
        ThrowIfFailed(compiler->Compile(&sourceBuffer, args.data(), (UINT32)args.size(),
                            includeHandler.Get(), IID_PPV_ARGS(&result)));
        
        // 检查错误
        ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        
        if (errors && errors->GetStringLength() > 0)
        {
            std::string errorMsg = "Shader compilation failed:\n";
            errorMsg += errors->GetStringPointer();
            ThrowErrorMessage(errorMsg.c_str());
            return false;
        }
        
        // 获取字节码
        ComPtr<IDxcBlob> bytecodeBlob;
        ThrowIfFailed(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&bytecodeBlob), nullptr));
        
        if (!bytecodeBlob)
        {
            ThrowErrorMessage("Failed to get compiled bytecode");
            return false;
        }
        
        outBytecode.resize(bytecodeBlob->GetBufferSize());
        memcpy(outBytecode.data(), bytecodeBlob->GetBufferPointer(), bytecodeBlob->GetBufferSize());
        return true;
    }


    std::shared_ptr<RHIVertexShader> RHIDirectX12::CompileVertexShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<VertexShaderDirectX12>(this, desc, GetShaderTarget("vs").c_str());
    }

    std::shared_ptr<RHIPixelShader> RHIDirectX12::CompilePixelShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<PixelShaderDirectX12>(this, desc, GetShaderTarget("ps").c_str());
    }

    std::shared_ptr<RHIGeometryShader> RHIDirectX12::CompileGeometryShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderDirectX12>(this, desc, GetShaderTarget("gs").c_str());
    }

    std::shared_ptr<RHIHullShader> RHIDirectX12::CompileHullShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<HullShaderDirectX12>(this, desc, GetShaderTarget("hs").c_str());
    }

    std::shared_ptr<RHIDomainShader> RHIDirectX12::CompileDomainShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<DomainShaderDirectX12>(this, desc, GetShaderTarget("ds").c_str());
    }

    std::shared_ptr<RHIComputeShader> RHIDirectX12::CompileComputeShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderDirectX12>(this, desc, GetShaderTarget("cs").c_str());
    }
}
