// RHI/SPIRVHelper.h
#pragma once

#include "IRHIModule.h"
#include <CoreMinimal.h>
#include <Tools/Singleton.h>

// Windows headers
#include <windows.h>

// Standard headers
#include <cstdint>
#include <memory>

// DXC headers
#include <include/dxcapi.h>
// COM headers
#include <wrl/client.h>
using namespace Microsoft::WRL;

namespace RHI {

// Shader compiler context
struct ShaderCompilerContext {
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcIncludeHandler> includeHandler;

    bool Initialize() {
        // Create DXC compiler and utils
        if (FAILED(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)))) {
            return false;
        }
        if (FAILED(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)))) {
            return false;
        }
        if (FAILED(utils->CreateDefaultIncludeHandler(&includeHandler))) {
            return false;
        }
        return true;
    }
};

// HLSL → SPIR-V Compiler
class HLSLToSPIRVCompiler : public IShaderCompiler , public Singleton<HLSLToSPIRVCompiler> {
public:
    HLSLToSPIRVCompiler();
    ~HLSLToSPIRVCompiler();

    // Compile from source string
    ShaderCompileResult CompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptions& options
    ) override;

    // Compile from file
    ShaderCompileResult CompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptions& options
    ) override;

private:
    // Internal compile function
    ShaderCompileResult CompileInternal(
        const std::string& hlslSource,
        const ShaderCompileOptions& options
    );

    // Build DXC arguments
    std::vector<const wchar_t*> BuildArguments(
        const ShaderCompileOptions& options,
        std::vector<std::wstring>& m_ArgStorage
    );

    // Get SPIR-V target env (call external function)
    std::string GetSPIRVTargetEnv() const {
        switch (GetBestAvailableRHI()) {
        case RHIType::D3D12:
            return "vulkan1.3";
        case RHIType::D3D11:
            return "vulkan1.0";
        default:
            return "vulkan1.0";
        }
    }

private:
    std::unique_ptr<ShaderCompilerContext> m_Context;
    bool m_Initialized = false;
};

/*
 * @brief SPIR-V Reflection
 * This class is used to load SPIR-V files from disk or memory
*/
class SPIRVGenerationReflection : public IShaderCompiler , public Singleton<SPIRVGenerationReflection> {
public:
    // Extract reflection information (using SPIRV-Cross)
    SPIRVReflection ExtractReflection(const std::vector<uint32_t>& spirv) override;
};

} // namespace RHI
