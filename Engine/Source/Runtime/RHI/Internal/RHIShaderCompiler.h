// RHI/SPIRVHelper.h
#pragma once

#include "IRHIModule.h"
#include "Common/RHIConfig.h"
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

/*
 * This is Shader compiler
 * Our shaders support multithreading, and what we’re doing just works on a single thread. For multithreading,
 * we use ContextController to manage different compiler contexts for each thread, so you can see I didn’t use any locks,
 * and we use static thread_local to isolate different threads.
*/

namespace RHI {

namespace Internal {
    IShaderCompiler* GetHLSLCompiler();
    IShaderCompiler* GetSPIRVCompiler();
    IShaderCompiler* GetSPIRVReflection();
}

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
        
        // Initialize success
        m_Initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_Initialized; }
    private:
        bool m_Initialized = false;
};

// Struct for internal compilation settings
struct LocalShaderCompileOption : public ShaderCompileOptions {
    // Target Compiler mode (SPIR-V or HLSL)
    std::string targetCompilerMode = "-spirv";
    // Target SPIR-V environment (vulkan1.0 or vulkan1.3)
    std::string SPIR_V_TargetEnv = "vulkan1.0";

    LocalShaderCompileOption() = default;
    LocalShaderCompileOption(const ShaderCompileOptions& options) : ShaderCompileOptions(options) {}
};

// Struct for compiler pipeline cache
struct CompilerPipelineCache {
    // Cache for compiler
    ShaderCompileResult CompileCache;
    // Cache for SPIRV reflection
    SPIRVReflection SPIRVReflectionCache;
    // Cache for Compiler Options
    LocalShaderCompileOption CompilerOptionsCache;
    // Cache for Source
    ShaderCompileSource SourceCache;

    void Clear() {
        CompileCache = ShaderCompileResult {};
        SPIRVReflectionCache = SPIRVReflection {};
        CompilerOptionsCache = LocalShaderCompileOption {};
        SourceCache = ShaderCompileSource {};
    }
};

// ----------------------------------------------------------------- Compiler Functions

// Internal compile function
[[nodiscard]] ShaderCompileResult CompileInternal(
    const std::string& hlslSource,
    const LocalShaderCompileOption& options,
    const ShaderCompilerContext& context
);

// Build DXC arguments
[[nodiscard]] std::vector<const wchar_t*> BuildArguments(
    const LocalShaderCompileOption& options,
    std::vector<std::wstring>& m_ArgStorage
);

// Get SPIR-V target env (call external function)
inline std::string GetSPIRVTargetEnv() {
    switch (GetBestAvailableRHI()) {
    case RHIType::D3D12:
        return "vulkan1.3";
    case RHIType::D3D11:
        return "vulkan1.0";
    default:
        return "vulkan1.0";
    }
}

// ----------------------------------------------------------------- Compiler Functions End

class CompilerContextController : public IShaderCompiler {
public:
    enum class CompilerContextState {
        Initialized,
        Shutdown
    };

    enum class CompilerPipelineState {
        Start,
        End
    };
public:
    CompilerContextController();
    ~CompilerContextController();

    // Compiler Context Controller
    bool InitializeCompilerContext() override;
    void ShutdownCompilerContext() override;

    const ShaderCompilerContext* GetCompilerContext() const { return m_Context.get(); }

    const CompilerPipelineCache* GetCompilerPipelineCache() const { return m_Cache.get(); }
    CompilerPipelineCache& AppendCompilerPipelineCache() { return *m_Cache; }
    // Normally we shouldn't use this function to set the cache because it will clear all existing caches unless you know what you're doing.
    void SetCompilerPipelineCache(const CompilerPipelineCache& cache) { *m_Cache = cache; }
    void ClearCompilerPipelineCache() { m_Cache->Clear(); }

    void SetCompilerPipelineState(CompilerPipelineState state) { m_Pipeline_State = state; }
    CompilerPipelineState GetCompilerPipelineState() const { return m_Pipeline_State; }
private:
    CompilerContextState m_State = CompilerContextState::Shutdown;
    CompilerPipelineState m_Pipeline_State = CompilerPipelineState::End;

    std::unique_ptr<ShaderCompilerContext> m_Context;
    std::unique_ptr<CompilerPipelineCache> m_Cache;
};

// Get local thread compiler context
[[nodiscard]] inline const ShaderCompilerContext* GetLocalThreadCompilerContext(const IShaderCompiler* compilerContext) {
    const CompilerContextController* controller = SafeCast<const CompilerContextController>(compilerContext);
    if (controller == nullptr) {
        // Compiler context not initialized, return error
#if RHI_ENABLE_DEBUG_INFO
        ThrowErrorMessage("Shader compiler context not initialized!");
#endif
        return nullptr;
    }
    const ShaderCompilerContext* context = controller->GetCompilerContext();
    if (context == nullptr) {
        // Compiler context not initialized, return error
#if RHI_ENABLE_DEBUG_INFO
        ThrowErrorMessage("Shader compiler context not initialized!");
#endif
        return nullptr;
    }
    return context;
}

class CompilerPipeline : public IShaderCompiler , public Singleton<CompilerPipeline> {
public:
    CompilerPipeline();
    ~CompilerPipeline();

    // State machine
    void BeginCompiler() override;
    void EndCompiler() override;

    // Real work function
    ShaderCompileResult Compile(const ShaderCompileOptions& options, const ShaderCompileSource& source) override;
    SPIRVReflection Reflection() override;
    CreateShaderDesc CreateShaderDescription() override;

private:
    ShaderCompileResult CompileD3D12(const ShaderCompileOptions& options, const ShaderCompileSource& source);
    ShaderCompileResult CompileOrdinaryAPI(const ShaderCompileOptions& options, const ShaderCompileSource& source);

private:

};

// HLSL → SPIR-V Compiler
class HLSLToSPIRVCompiler : public IShaderCompiler , public Singleton<HLSLToSPIRVCompiler> {
public:
    HLSLToSPIRVCompiler();
    ~HLSLToSPIRVCompiler();
protected:
    // Compile from source string
    ShaderCompileResult SPIRVCompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptions& options
    ) override;

    // Compile from file
    ShaderCompileResult SPIRVCompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptions& options
    ) override;

private:
};

// HLSL Compiler
class HLSLCompiler : public IShaderCompiler , public Singleton<HLSLCompiler> {
public:
    HLSLCompiler();
    ~HLSLCompiler();
protected:
    // Compile from source string
    ShaderCompileResult HLSLCompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptions& options
    ) override;

    // Compile from file
    ShaderCompileResult HLSLCompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptions& options
    ) override;
private:
};

/*
 * @brief SPIR-V Reflection
 * This class is used to load SPIR-V files from disk or memory
*/
class SPIRVGenerationReflection : public IShaderCompiler , public Singleton<SPIRVGenerationReflection> {
public:
    SPIRVGenerationReflection();
    ~SPIRVGenerationReflection();
protected:
    // Extract reflection information (using SPIRV-Cross)
    SPIRVReflection ExtractReflection(const std::vector<uint32_t>& spirv) override;
private:
};

} // namespace RHI
