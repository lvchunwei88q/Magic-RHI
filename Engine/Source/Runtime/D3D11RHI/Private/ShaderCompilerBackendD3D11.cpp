#include "RHIResourceD3D11.h"
#include <Common/RHIDefinitions.h>
#include <Common/Check.h>
#include "RHID3D11.h"
#include <d3dcompiler.h>
#include <spirv_hlsl.hpp>
#include <spirv_common.hpp>

#include <IO.h>

namespace RHI
{
    namespace {
        std::vector<uint32_t> ConvertUint8ToUint32(const std::vector<uint8_t>& byteCode) {
            // Check the alignment of the byte code
            if (byteCode.size() % sizeof(uint32_t) != 0) {
                ThrowErrorMessage("Bytecode size is not a multiple of 4");
            }
            
            const uint32_t* data = reinterpret_cast<const uint32_t*>(byteCode.data());
            size_t count = byteCode.size() / sizeof(uint32_t);
            return std::vector<uint32_t>(data, data + count);
        }

        /**
        * @brief Converts any shader target profile string to its corresponding 5_0 version
        * @param targetProfile Input target profile, e.g., "vs_6_x", "ps_5_1", "cs_x_x"
        * @return Converted profile string, e.g., "vs_5_0", "ps_5_0", "cs_5_0"
        */
        std::string ConvertToShaderModel50(const std::string& targetProfile)
        {
            // Return default vertex shader profile if input is empty
            if (targetProfile.empty()) {
                return "vs_5_0";
            }

            // Find the first underscore to extract shader type prefix
            size_t pos = targetProfile.find('_');
            if (pos != std::string::npos) {
                std::string shaderType = targetProfile.substr(0, pos);
                // Validate that it's a recognized shader type
                if (shaderType == "vs" || shaderType == "ps" || shaderType == "cs" ||
                    shaderType == "gs" || shaderType == "hs" || shaderType == "ds" ||
                    shaderType == "lib") {
                    return shaderType + "_5_0";
                }
            }
            // Fallback for invalid or unrecognized input
            return "vs_5_0";
        }

        bool CompileShaderToBlob(const std::string& source, const ShaderCompileOptions& options,
                            ComPtr<ID3DBlob>& outBlob, std::string& errorMsg)
        {
            UINT flags = 0;
            if (options.debugInfo)
            {
                flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
            }

            std::vector<D3D_SHADER_MACRO> dx11Macros;
            for (const auto& macro : options.Macros)
                dx11Macros.push_back({macro.name.c_str(), macro.definition.c_str()});
            // Add nullptr to mark the end of the macro list
            dx11Macros.push_back({nullptr, nullptr});

            // Convert target profile to shader model 5_0
            std::string shaderModel50 = ConvertToShaderModel50(options.targetProfile);

            ComPtr<ID3DBlob> errorBlob;
            HRESULT hr = D3DCompile(
                source.c_str(),
                source.size(),
                nullptr,
                dx11Macros.data(),
                nullptr,
                options.entryPoint.c_str(),
                shaderModel50.c_str(),
                flags,
                0,
                outBlob.GetAddressOf(),
                errorBlob.GetAddressOf()
            );

            if (FAILED(hr))
            {   
                errorMsg = "Shader compilation failed";

                // add HRESULT info
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
                    // When there’s no detailed error info, provide a hint based on the HRESULT
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

                return false;
            }
            return true;
        }
    }
    bool ShaderCompilerBackendD3D11::Initialize()
    {
        m_Initialization = InitialState::Initialize;
        return true;
    }

    void ShaderCompilerBackendD3D11::Shutdown()
    {
        m_Initialization = InitialState::Shutdown;
    }

    bool ShaderCompilerBackendD3D11::IsValid() const
    {
        return m_Initialization == InitialState::Initialize;
    }

    ShaderCompileOptionInternal ShaderCompilerBackendD3D11::AddBackendArguments(const ShaderCompileOptions& options)
    {
        ShaderCompileOptionInternal internalOptions = options;
        // The target compiler for DXC SPIR-V is -spirv
        internalOptions.targetCompilerMode = "-spirv";
        internalOptions.SPIR_V_TargetEnv = SPIRVCompileEnvironment().c_str();
        return internalOptions;
    }

    void ShaderCompilerBackendD3D11::PostProcessShader(const ShaderCompileOptions& options, const ShaderCompileResult& in_result, ShaderCompileResult& out_result)
    {
        std::vector<uint32_t> spirv = ConvertUint8ToUint32(in_result.byteCode);

        // create SPIRV-Cross HLSL compiler
        spirv_cross::CompilerHLSL hlslCompiler(std::move(spirv));
        // set compile options
        spirv_cross::CompilerGLSL::Options optionsGLSL;
        // Force uninitialized variables to 0, otherwise FXC might throw an error.
        optionsGLSL.force_zero_initialized_variables = true;

        spirv_cross::CompilerHLSL::Options optionsHLSL;
        // set Shader Model, 50 is the default
        optionsHLSL.shader_model = 50; 
        

        hlslCompiler.set_common_options(optionsGLSL);
        hlslCompiler.set_hlsl_options(optionsHLSL);

        // TODO: add HLSL-specific resource binding mappings
        // ...
        
        // compile to HLSL source
        std::string hlslSource;
        try {
            hlslSource = hlslCompiler.compile();
        } catch (const std::exception& e) {
            // catch SPIRV-Cross error
            Core::ErrorCapture::Capture(std::string("SPIRV-Cross compilation failed: ") + e.what());
            out_result.errorMessage = e.what();
            out_result.success = false;
        }

        std::string errorMsg;
        ComPtr<ID3DBlob> blob;
        if (!CompileShaderToBlob(hlslSource, options, blob, errorMsg))
        {
            out_result.errorMessage = errorMsg;
            out_result.success = false;
            return;
        }

        std::vector<uint8_t> bytecode;
        bytecode.resize(blob->GetBufferSize());
        memcpy(bytecode.data(), blob->GetBufferPointer(), blob->GetBufferSize());

        // set output result
        out_result.byteCode = bytecode;
        out_result.bytecodeSize = bytecode.size();
        out_result.success = true;
    }

    ShaderReflectionGenerationMode ShaderCompilerBackendD3D11::GetShaderReflectionGenerationMode()
    {
        return ShaderReflectionGenerationMode{ShaderReflectionGenerationMode::ReflectionGenerationMode::Use_InitialCompileCache};
    }

    // ==================================================== Tools ====================================================
    std::string ShaderCompilerBackendD3D11::SPIRVCompileEnvironment() const
    {
        return "vulkan1.0";
    }
}
