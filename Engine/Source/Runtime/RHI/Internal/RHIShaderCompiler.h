// RHI/SPIRVHelper.h
#pragma once

#include "IRHIModule.h"
#include "Common/RHIConfig.h"
#include <CoreMinimal.h>
#include <Tools/Singleton.h>
#include "RHIInterface.h"

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
    IShaderCompiler* GetCompilerInstance();
    IShaderCompiler* GetSPIRVReflectionGenerator();
}

// Shader compiler context
struct LocalShaderCompilerContext {
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcIncludeHandler> includeHandler;
    mutable std::unique_ptr<ShaderCompilerBackend> m_Backend;

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
        // Create shader compiler backend API
        m_Backend = IRHIModule::GetRHILoader()->CreateShaderCompilerBackend();
        if (m_Backend == nullptr || !m_Backend->Initialize()) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Shader compiler backend initialize failed!");
#endif
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

// Struct for compiler pipeline cache
struct LocalCompilerPipelineCache {
    // Cache for compiler compile result
    ShaderCompileResult InitialCompileCache;
    // Cache for compiler compile result
    ShaderCompileResult CompileResultCache;

    // Cache for SPIRV reflection
    SPIRVReflection SPIRVReflectionCache;
    // Cache for Compiler Options
    ShaderCompileOptionInternal CompilerOptionsCache;
    // Cache for Source
    ShaderCompileSource SourceCache;
    
    void Clear() {
        InitialCompileCache = ShaderCompileResult {};
        CompileResultCache = ShaderCompileResult {};

        SPIRVReflectionCache = SPIRVReflection {};
        CompilerOptionsCache = ShaderCompileOptionInternal {};
        SourceCache = ShaderCompileSource {};
    }
};

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
    bool InitializeCompilerThreadContext() override;
    void ShutdownCompilerThreadContext() override;

    // Get compiler context and backend
    const LocalShaderCompilerContext* GetCompilerContext() const { return m_Context.get(); }

    const LocalCompilerPipelineCache* GetCompilerPipelineCache() const { return m_Cache.get(); }
    LocalCompilerPipelineCache& AppendCompilerPipelineCache() { return *m_Cache; }
    // Normally we shouldn't use this function to set the cache because it will clear all existing caches unless you know what you're doing.
    void SetCompilerPipelineCache(const LocalCompilerPipelineCache& cache) { *m_Cache = cache; }
    void ClearCompilerPipelineCache() { m_Cache->Clear(); }

    void SetCompilerPipelineState(CompilerPipelineState state) { m_Pipeline_State = state; }
    CompilerPipelineState GetCompilerPipelineState() const { return m_Pipeline_State; }
private:
    CompilerContextState m_ThreadContext_State = CompilerContextState::Shutdown;
    CompilerPipelineState m_Pipeline_State = CompilerPipelineState::End;

    std::unique_ptr<LocalShaderCompilerContext> m_Context;
    std::unique_ptr<LocalCompilerPipelineCache> m_Cache;
};

// Get local thread compiler context
[[nodiscard]] inline const LocalShaderCompilerContext* GetLocalThreadCompilerContext(const IShaderCompiler* compilerContext) {
    const CompilerContextController* controller = SafeCast<const CompilerContextController>(compilerContext);
    if (controller == nullptr) {
        // Compiler context not initialized, return error
#if RHI_ENABLE_DEBUG_INFO
        ThrowErrorMessage("Shader compiler context not initialized!");
#endif
        return nullptr;
    }
    const LocalShaderCompilerContext* context = controller->GetCompilerContext();
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

};

// HLSL → SPIR-V Compiler or HLSL Compiler
class CompilerInstance : public IShaderCompiler , public Singleton<CompilerInstance> {
public:
    CompilerInstance();
    ~CompilerInstance();
protected:
    // Compile from source string
    ShaderCompileResult CompileFromString(
        const std::string& hlslSource,
        const ShaderCompileOptionInternal& options
    ) override;

    // Compile from file
    ShaderCompileResult CompileFromFile(
        const std::string& filePath,
        const ShaderCompileOptionInternal& options
    ) override;

private:
    // compile function
    [[nodiscard]] ShaderCompileResult CompileInternal(
        const std::string& hlslSource,
        const ShaderCompileOptionInternal& options,
        const LocalShaderCompilerContext& context
    );

    // Build DXC arguments
    [[nodiscard]] std::vector<const wchar_t*> BuildArguments(
        const ShaderCompileOptionInternal& options,
        std::vector<std::wstring>& m_ArgStorage
    );
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
