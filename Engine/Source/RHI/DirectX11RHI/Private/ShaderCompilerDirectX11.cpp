#include "RHIResourceDirectX11.h"
#include "RHIDirectX11.h"
#include "DirectXHelper.h"
#include <d3dcompiler.h>
#include <regex>

#include <Converter.h>
#include <FileManager.h>

#include <CoreLogCapture/CoreLogCapture.h>

namespace RHI
{
    namespace
    {
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

        std::string ProcessShaderIncludes(const std::string& source, const std::string& basePath)
        {
            static std::regex includeRegex(R"(#include\s*["<]([^">]+)[">])");
            std::string result;
            size_t pos = 0;
            std::string temp = source;
            std::smatch match;
            
            while (std::regex_search(temp, match, includeRegex))
            {
                // 保留 #include 前面的部分
                result += match.prefix().str();
            
                std::string includePath = match[1].str();
                std::string fullPath = basePath + "\\" + includePath;
                std::string includeContent = ReadFileToString(fullPath);
                
                result += ProcessShaderIncludes(includeContent, basePath);
                
                temp = match.suffix().str();
            }
            
            result += temp;
            
            return result;
        }

        bool CompileShaderToBlob(const std::string& source, const std::string& entryPoint, 
                                const std::string& profile, bool enableDebug,
                                ComPtr<ID3DBlob>& outBlob)
        {
            UINT flags = 0;
#ifdef _DEBUG
            flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            if (enableDebug)
            {
                flags |= D3DCOMPILE_DEBUG;
            }

            ComPtr<ID3DBlob> errorBlob;
            HRESULT hr = D3DCompile(
                source.c_str(),
                source.size(),
                nullptr,
                nullptr,
                nullptr,
                entryPoint.c_str(),
                profile.c_str(),
                flags,
                0,
                outBlob.GetAddressOf(),
                errorBlob.GetAddressOf()
            );

            if (FAILED(hr))
            {
                std::string errorMsg = "Shader compilation failed";
                
                // 添加 HRESULT 信息
                char hrMsg[64];
                sprintf_s(hrMsg, " (HRESULT: 0x%08X)", static_cast<unsigned int>(hr));
                errorMsg += hrMsg;
                errorMsg += ":\n";
                
                if (errorBlob)
                {
                    errorMsg += reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
                }
                else
                {
                    // 没有详细错误信息时，根据 HRESULT 提供提示
                    switch (hr)
                    {
                    case E_OUTOFMEMORY:
                        errorMsg += "Out of memory";
                        break;
                    case E_INVALIDARG:
                        errorMsg += "Invalid argument (check shader target or entry point)";
                        break;
                    default:
                        errorMsg += "Unknown error";
                        break;
                    }
                }
                
                Core::ErrorCapture::Capture(errorMsg.c_str());
                // ThrowErrorMessage(errorMsg.c_str());   这里不要直接抛出异常，否则会导致程序崩溃
                return false;
            }

            return true;
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
                std::string source = ReadFileToString(filePath);
                return ProcessShaderIncludes(source, outBasePath);
            }
            else
            {
                ThrowErrorMessage("No shader source or file path provided");
                return "";
            }
        }

        template<typename ShaderType, typename CreateFunc>
        std::shared_ptr<ShaderType> CompileShaderInternal(ID3D11Device* pDevice, const ShaderCompileDesc& desc,
                                                          const char* defaultProfile, CreateFunc createFunc)
        {
            std::string basePath;
            std::string source = LoadShaderSource(desc, basePath);
            if (source.empty())
                return nullptr;

            std::string profile = desc.Profile ? desc.Profile : defaultProfile;
            std::string entryPoint = desc.EntryPoint ? desc.EntryPoint : "main";

            ComPtr<ID3DBlob> shaderBlob;
            if (!CompileShaderToBlob(source, entryPoint, profile, desc.EnableDebugInfo, shaderBlob))
                return nullptr;

            auto pShader = createFunc(pDevice, shaderBlob);
            if (!pShader)
                return nullptr;

            return std::make_shared<ShaderType>(pShader);
        }
    }

    std::shared_ptr<RHIVertexShader> RHIDirectX11::CompileVertexShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<VertexShaderDirectX11>(m_pDevice.Get(), desc, "vs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11VertexShader* {
                ID3D11VertexShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::shared_ptr<RHIPixelShader> RHIDirectX11::CompilePixelShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<PixelShaderDirectX11>(m_pDevice.Get(), desc, "ps_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11PixelShader* {
                ID3D11PixelShader* pShader = nullptr;
                if (SUCCEEDED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::shared_ptr<RHIGeometryShader> RHIDirectX11::CompileGeometryShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<GeometryShaderDirectX11>(m_pDevice.Get(), desc, "gs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11GeometryShader* {
                ID3D11GeometryShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::shared_ptr<RHIHullShader> RHIDirectX11::CompileHullShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<HullShaderDirectX11>(m_pDevice.Get(), desc, "hs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11HullShader* {
                ID3D11HullShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::shared_ptr<RHIDomainShader> RHIDirectX11::CompileDomainShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<DomainShaderDirectX11>(m_pDevice.Get(), desc, "ds_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11DomainShader* {
                ID3D11DomainShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }

    std::shared_ptr<RHIComputeShader> RHIDirectX11::CompileComputeShader(const ShaderCompileDesc& desc)
    {
        return CompileShaderInternal<ComputeShaderDirectX11>(m_pDevice.Get(), desc, "cs_5_0",
            [](ID3D11Device* device, ComPtr<ID3DBlob>& blob) -> ID3D11ComputeShader* {
                ID3D11ComputeShader* pShader = nullptr;
                if (SUCCEEDED(device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader)))
                    return pShader;
                return nullptr;
            });
    }
}
