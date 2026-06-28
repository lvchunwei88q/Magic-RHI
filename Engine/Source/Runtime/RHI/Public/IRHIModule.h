// ========== RHI/Public/IRHILoader.h ==========

#pragma once
#include <memory>
#include "Common/RHI_API.h"
#include "Common/RHIDefinitions.h"
#include "Common/RHIPlatformDetection.h"

namespace RHI
{
    // Forward Declaration
    class Device;
    class SwapChain;
    class CreateShader;

    // RHI Loader Interface
    class IRHILoader;
    // SPIRV Processor Interface
    class IShaderCompiler;

    // RHI Module Interface
    class RHI_API IRHIModule
    {
    public:
        virtual ~IRHIModule() = default;

        static IRHILoader* GetRHILoader();

        static std::unique_ptr<IShaderCompiler> GetCompilerContextController();
        static IShaderCompiler* GetCompilerPipeline();
    };

    class RHI_API IRHILoader
    {
    public:
        virtual ~IRHILoader() = default;

        virtual bool Load(RHIType type) = 0;
        virtual void Unload() = 0;
        
        virtual std::unique_ptr<Device> CreateDevice() = 0;
        virtual std::unique_ptr<SwapChain> CreateSwapChain() = 0;
        virtual std::unique_ptr<CreateShader> CreateCreateShader() = 0;
        
        virtual bool IsLoaded() const = 0;
        virtual RHIType GetRHIType() const = 0;
    };

    // Core subclass of IShaderCompiler
    class CompilerPipeline;
    class RHI_API IShaderCompiler 
    {
    public:
        virtual ~IShaderCompiler () = default;

        // Compiler Context Controller
        virtual bool InitializeCompilerContext() { return false; }
        virtual void ShutdownCompilerContext() {}

        // ----------------------------------------------------------------- Compiler Functions Start
        // Compiler Core
        /*
        * Because we used some cache optimization tricks,
        * we adopted a state machine pattern to control buffering—every time we compile,
        * we have to call BeginCompiler first and then call EndCompiler at the end.
        */
        virtual void BeginCompiler() {}
        virtual void EndCompiler() {}

        // Real work function
        virtual ShaderCompileResult Compile(const ShaderCompileOptions& options, const ShaderCompileSource& source) { return ShaderCompileResult {}; }
        virtual SPIRVReflection Reflection() { return SPIRVReflection {}; }
        virtual CreateShaderDesc CreateShaderDescription() { return CreateShaderDesc {}; }
        
    protected:
        // Compile HLSL
        virtual ShaderCompileResult HLSLCompileFromString(const std::string& hlslSource, const ShaderCompileOptions& options) { return ShaderCompileResult {}; }
        virtual ShaderCompileResult HLSLCompileFromFile(const std::string& filePath, const ShaderCompileOptions& options) { return ShaderCompileResult {}; };

        // Compile HLSL → SPIR-V
        virtual ShaderCompileResult SPIRVCompileFromString(const std::string& hlslSource, const ShaderCompileOptions& options) { return ShaderCompileResult {}; }
        virtual ShaderCompileResult SPIRVCompileFromFile(const std::string& filePath, const ShaderCompileOptions& options) { return ShaderCompileResult {}; };
        
        // Extract reflection information
        virtual SPIRVReflection ExtractReflection(const std::vector<uint32_t>& spirv) { return SPIRVReflection {}; }

        // Compiler Context
        void SetCompilerContext(IShaderCompiler* compilerContext) { GetSharedCompilerContext() = compilerContext; }
        IShaderCompiler* GetCompilerContext() const { return GetSharedCompilerContext(); }

    private:
        static IShaderCompiler*& GetSharedCompilerContext() {
            // thread_local Independent copy for each thread
            static thread_local IShaderCompiler* compiler_context = nullptr; 
            return compiler_context;
        }

        friend class CompilerPipeline;
    };

} // namespace RHI

using RHIModule = RHI::IRHIModule;
using RHIAPILoader = RHI::IRHILoader;
using RHIShaderCompiler = RHI::IShaderCompiler;
