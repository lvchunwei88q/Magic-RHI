#include "Common/Check.h"
#include "CoreMinimal.h"
#include "IRHIModule.h"
#include "RHIShaderCompiler.h"
#include "RHILoader.h"

namespace RHI {
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
    }

    // ========== Compiler Pipeline ==========
    // Get compiler pipeline instance
    IShaderCompiler* IRHIModule::GetCompilerPipeline(){
        return &CompilerPipeline::Get();
    }

    CompilerPipeline::CompilerPipeline() = default;
    CompilerPipeline::~CompilerPipeline() = default;

    // With DX12, we can directly compile HLSL shaders without any extra steps.
    ShaderCompileResult CompilerPipeline::CompileD3D12(const ShaderCompileOptions& options, const ShaderCompileSource& source) {
        // D3D12 compiler pipeline
        IShaderCompiler* HLSLCompiler = Internal::GetHLSLCompiler();
        ShaderCompileResult result;

        switch (source.sourceType) {
        case ShaderCompileSource::SourceType::SourcePath:
            result = HLSLCompiler->HLSLCompileFromFile(source.sourceDescription, options);
            break;
        case ShaderCompileSource::SourceType::SourceCode:
            result = HLSLCompiler->HLSLCompileFromString(source.sourceDescription, options);
            break;
        default:
            result.errorMessage = "Invalid source type";
            result.success = false;
            break;
        }

        return result;
    }

    // When we're using APIs that aren't DX12, we need to first compile the shaders into SPIRV bytecode using a SPIRV compiler, and then pass it on to the DX11 backend for processing.
    ShaderCompileResult CompilerPipeline::CompileOrdinaryAPI(const ShaderCompileOptions& options, const ShaderCompileSource& source) {
        // D3D11 compiler pipeline
        IShaderCompiler* SPIRVCompiler = Internal::GetSPIRVCompiler();
        ShaderCompileResult result;

        switch (source.sourceType) {
        case ShaderCompileSource::SourceType::SourcePath:
            result = SPIRVCompiler->SPIRVCompileFromFile(source.sourceDescription, options);
            break;
        case ShaderCompileSource::SourceType::SourceCode:
            result = SPIRVCompiler->SPIRVCompileFromString(source.sourceDescription, options);
            break;
        default:
            result.errorMessage = "Invalid source type";
            result.success = false;
            break;
        }

        return result;
    }

    // State machine
    void CompilerPipeline::BeginCompiler() {
        IShaderCompiler* compilerContext = GetCompilerContext();
        CompilerContextController* controller = SafeCast<CompilerContextController>(compilerContext);
        if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Compiler context controller is null");
#endif
        }

        if(controller->GetCompilerPipelineState() != CompilerContextController::CompilerPipelineState::End){
            ThrowErrorMessage("The compiler state machine hasn't finished yet, so don't call it again.");
            return;
        }
        controller->SetCompilerPipelineState(CompilerContextController::CompilerPipelineState::Start);
    }

    void CompilerPipeline::EndCompiler() {
        IShaderCompiler* compilerContext = GetCompilerContext();
        CompilerContextController* controller = SafeCast<CompilerContextController>(compilerContext);
        if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Compiler context controller is null");
#endif
        }
        controller->ClearCompilerPipelineCache();
        if(controller->GetCompilerPipelineState() == CompilerContextController::CompilerPipelineState::End){
            ThrowErrorMessage("The compiler state machine is already shut down, so don't call it anymore..");
            return;
        }
        controller->SetCompilerPipelineState(CompilerContextController::CompilerPipelineState::End);
    }

    ShaderCompileResult CompilerPipeline::Compile(const ShaderCompileOptions& options, const ShaderCompileSource& source) {
        // Get compilerContext
        IShaderCompiler* compilerContext = GetCompilerContext();
        CompilerContextController* controller = SafeCast<CompilerContextController>(compilerContext);
        if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Compiler context controller is null");
#endif
            // Default implementation
            return {};
        }
        
        // Default implementation
        ShaderCompileResult result;
        result.success = false;
        result.errorMessage = "Not implemented";

        // Check API type
        RHIType APIType = RHILoader::Get().GetRHIType();
        // Compile shader based on API type
        switch (APIType) {
        // Only DX12 requires us to handle it specially
        case RHIType::D3D12:
            // D3D12 compiler pipeline
            result = CompileD3D12(options, source);
            break;
        default:result = CompileOrdinaryAPI(options, source);
            break;
        }

        // Set cache Because this is the initial compile, it should be the start of all cached data.
        LocalShaderCompileOption CompilerOptionCache = LocalShaderCompileOption(options);
        CompilerOptionCache.SPIR_V_TargetEnv = GetSPIRVTargetEnv();
        controller->AppendCompilerPipelineCache().CompileCache = result;
        controller->AppendCompilerPipelineCache().SourceCache = source;
        controller->AppendCompilerPipelineCache().CompilerOptionsCache = CompilerOptionCache;

        return result;
    }

    SPIRVReflection CompilerPipeline::Reflection() {
        // Check API type
        RHIType APIType = RHILoader::Get().GetRHIType();

        // Get compilerContext
        IShaderCompiler* compilerContext = GetCompilerContext();
        CompilerContextController* controller = SafeCast<CompilerContextController>(compilerContext);
        if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Compiler context controller is null");
#endif
            // Default implementation
            return {};
        }

        // Get cache
        const CompilerPipelineCache* cache = controller->GetCompilerPipelineCache();

        ShaderCompileResult SPIRVCompileResult;

        auto SPIRVCompileLambda = [cache]() -> ShaderCompileResult {
            std::string sourceDescription = cache->SourceCache.sourceDescription;
            switch (cache->SourceCache.sourceType) {
                case ShaderCompileSource::SourceType::SourcePath:
                    return Internal::GetSPIRVCompiler()->SPIRVCompileFromFile(sourceDescription, cache->CompilerOptionsCache);
                    break;
                case ShaderCompileSource::SourceType::SourceCode:
                    return Internal::GetSPIRVCompiler()->SPIRVCompileFromString(sourceDescription, cache->CompilerOptionsCache);
                    break;
                default: ThrowErrorMessage("Not implemented");
                    return {};
                    break;
            }
        };
        // Compile shader based on API type
        switch (APIType) {
        // Only DX12 requires us to handle it specially
        case RHIType::D3D12:
            // D3D12 compiler pipeline - SPI SPIRV
            SPIRVCompileResult = SPIRVCompileLambda();
            break;
            // use cache
        default: SPIRVCompileResult = controller->GetCompilerPipelineCache()->CompileCache;
            break;
        }

        // Check compile result
        if(!SPIRVCompileResult.success){
            Core::ErrorCapture::Capture(SPIRVCompileResult.errorMessage);
            // Default implementation
            struct SPIRVReflection reflection;
            return reflection;
        }

        std::vector<uint32_t> SPIRVByteCode = ConvertUint8ToUint32(SPIRVCompileResult.byteCode);
        // Extract reflection
        IShaderCompiler* SPIRVReflection = Internal::GetSPIRVReflection();
        if (!SPIRVReflection) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("SPIRV reflection is null");
#endif
            // Default implementation
            return {};
        }

        struct SPIRVReflection reflection = SPIRVReflection->ExtractReflection(SPIRVByteCode);
        // Set cache
        controller->AppendCompilerPipelineCache().SPIRVReflectionCache = reflection;
        return reflection;
    }

    CreateShaderDesc CompilerPipeline::CreateShaderDescription() {
        // Check API type
        RHIType APIType = RHILoader::Get().GetRHIType();
        // Get compilerContext
        IShaderCompiler* compilerContext = GetCompilerContext();
        CompilerContextController* controller = SafeCast<CompilerContextController>(compilerContext);
        if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Compiler context controller is null");
#endif
            // Default implementation
            return {};
        }

        // Get cache
        const CompilerPipelineCache* cache = controller->GetCompilerPipelineCache();
        CreateShaderDesc shaderDesc;
        shaderDesc.byteCode = cache->CompileCache.byteCode;

        // output error message and warning message
        if(cache->CompileCache.errorMessage.size() > 0){
            Core::ErrorCapture::Capture(cache->CompileCache.errorMessage);
        }
        if(cache->CompileCache.warningMessage.size() > 0){
            Core::WarningCapture::Capture(cache->CompileCache.warningMessage);
        }

        // Create shader description based on API type
        switch (APIType) {
        // Only DX12 requires us to handle it specially
        case RHIType::D3D12:
            shaderDesc.shaderType = CreateShaderDesc::ShaderType::HLSL;
            break;
        default:shaderDesc.shaderType = CreateShaderDesc::ShaderType::SPIRV;
            break;
        }

        return shaderDesc;
    }

}
