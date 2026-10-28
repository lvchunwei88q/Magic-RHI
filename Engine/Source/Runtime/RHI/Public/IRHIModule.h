// ========== RHI/Public/IRHILoader.h ==========

#pragma once
#include <memory>
#include "Common/RHI_API.h"
#include "Common/RHIDefinitions.h"
// Platform Check
#include "RHIPlatformDetection.h"

namespace RHI
{
    // Forward Declaration
    class Device;
    class SwapChain;
    class ShaderCompilerBackend;

    // RHI Loader Interface
    class IRHILoader;
    // SPIRV Processor Interface
    class IShaderCompiler;

    // RHI Module Interface
    class RHI_API IRHIModule
    {
    public:
        /**
         * @brief
         */
        virtual ~IRHIModule() = default;

        /**
         * @brief
         * @return
         */
        static IRHILoader* GetRHILoader();

        /**
         * @brief
         * @return
         */
        static std::unique_ptr<IShaderCompiler> GetCompilerContextController();
        /**
         * @brief
         * @return
         */
        static IShaderCompiler* GetCompilerPipeline();
    };

    class RHI_API IRHILoader
    {
    public:
        /**
         * @brief
         */
        virtual ~IRHILoader() = default;
        
        /**
         * @brief Loads a single RHI backend specified by the type.
         * @param type The backend to load. Must be a single RHI type (not a combination).
         * @return true on success, false if the type is invalid or initialization fails.
         * @note Although RHIType is a bitmask, this function does not handle multiple backends at once.
         * The caller should pre-select the optimal single backend to pass in.
         */
        virtual bool Load(RHIType type) = 0;

        /**
         * @brief Unloads the active RHI backend.
         */
        virtual void Unload() = 0;
        
        /**
         * @brief
         * @return
         */
        virtual std::unique_ptr<Device> CreateDevice() = 0;
        /**
         * @brief
         * @return
         */
        virtual std::unique_ptr<SwapChain> CreateSwapChain() = 0;
        /**
         * @brief
         * @return
         */
        virtual std::unique_ptr<ShaderCompilerBackend> CreateShaderCompilerBackend() = 0;
        
        /**
         * @brief
         * @return
         */
        virtual bool IsLoaded() const = 0;
        /**
         * @brief
         * @return
         */
        virtual RHIType GetRHIType() const = 0;
    };

    // Core subclass of IShaderCompiler
    class CompilerPipeline;
    class RHI_API IShaderCompiler 
    {
    public:
        /**
         * @brief
         */
        virtual ~IShaderCompiler () = default;

        // Compiler Context Controller
        /**
         * @brief
         * @return
         */
        virtual bool InitializeCompilerThreadContext() { return false; }
        /**
         * @brief
         */
        virtual void ShutdownCompilerThreadContext() {}

        // ----------------------------------------------------------------- Compiler Functions Start
        // Compiler Core
        /**
         * @brief Because we used some cache optimization tricks, we adopted a state machine pattern to control buffering.
         * Every time we compile, we have to call BeginCompiler first and then call EndCompiler at the end.
         */
        virtual void BeginCompiler() {}
        /**
         * @brief
         */
        virtual void EndCompiler() {}

        // Real work function
        /**
         * @brief
         * @param options
         * @param source
         * @return
         */
        virtual ShaderCompileResult Compile(const ShaderCompileOptions& options, const ShaderCompileSource& source) { return ShaderCompileResult {}; }
        /**
         * @brief
         * @return
         */
        virtual SPIRVReflection Reflection() { return SPIRVReflection {}; }
        /**
         * @brief
         * @return
         */
        virtual CreateShaderDesc CreateShaderDescription() { return CreateShaderDesc {}; }
        
    protected:
        // Compile HLSL → SPIR-V ByteCode or HLSL ByteCode
        /**
         * @brief
         * @param hlslSource
         * @param options
         * @return
         */
        virtual ShaderCompileResult CompileFromString(const std::string& hlslSource, const ShaderCompileOptionInternal& options) { return ShaderCompileResult {}; }
        /**
         * @brief
         * @param filePath
         * @param options
         * @return
         */
        virtual ShaderCompileResult CompileFromFile(const std::string& filePath, const ShaderCompileOptionInternal& options) { return ShaderCompileResult {}; };
        
        // Extract reflection information
        /**
         * @brief
         * @param spirv
         * @return
         */
        virtual SPIRVReflection ExtractReflection(const std::vector<uint32_t>& spirv) { return SPIRVReflection {}; }

        // Compiler Context
        /**
         * @brief
         * @param compilerContext
         */
        void SetCompilerContext(IShaderCompiler* compilerContext) { GetSharedCompilerContext() = compilerContext; }
        /**
         * @brief
         * @return
         */
        IShaderCompiler* GetCompilerContext() const { return GetSharedCompilerContext(); }

    private:
        /**
         * @brief
         * @return
         */
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
