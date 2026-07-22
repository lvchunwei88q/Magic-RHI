#include "Common/Check.h"
#include "CoreMinimal.h"
#include "IRHIModule.h"
#include "RHIShaderCompiler.h"

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

        bool CheckCompilerResult(ShaderCompileResult& result){
            if(result.success == false || result.errorMessage.size() > 0){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage(result.errorMessage);
#endif
                Core::ErrorCapture::Capture(result.errorMessage);
                Core::WarningCapture::Capture(result.errorMessage);
                return false;
            }
            // if warning message is not empty, then capture it
            if(result.warningMessage.size() > 0){
                Core::WarningCapture::Capture(result.warningMessage);
            }
            return true;
        }
    }

    // ========== Compiler Pipeline ==========
    // Get compiler pipeline instance
    IShaderCompiler* IRHIModule::GetCompilerPipeline(){
        return &CompilerPipeline::Get();
    }

    CompilerPipeline::CompilerPipeline() = default;
    CompilerPipeline::~CompilerPipeline() = default;

    // ============================================================= Set Compiler Pipeline State =============================================================
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
    // ============================================================= Compile =============================================================

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
        ShaderCompileResult InitialCompileResult;
        InitialCompileResult.success = false;
        InitialCompileResult.errorMessage = "Not implemented";

        // Get backend
        ShaderCompilerBackend* backend = controller->GetCompilerContext()->m_Backend.get();
        if (!backend || !backend->IsValid()) {
            ThrowErrorMessage("Shader compiler backend is null");
        }

        // Use ShaderCompilerBackend to generate parameters for a specific backend API
        ShaderCompileOptionInternal CompilerOptions = backend->AddBackendArguments(options);
        IShaderCompiler* compiler = Internal::GetCompilerInstance();
        if (!compiler) {    
            ThrowErrorMessage("Shader compiler instance is null");
        }

        // Compile shader from source
        if(source.sourceType == ShaderCompileSource::SourceType::SourcePath){
            InitialCompileResult = compiler->CompileFromFile(source.sourceDescription, CompilerOptions);
        }else if(source.sourceType == ShaderCompileSource::SourceType::SourceCode){
            InitialCompileResult = compiler->CompileFromString(source.sourceDescription, CompilerOptions);
        }else{
            ThrowErrorMessage("Not implemented");
            return {};
        }

        controller->AppendCompilerPipelineCache().InitialCompileCache = InitialCompileResult;
        controller->AppendCompilerPipelineCache().SourceCache = source;
        controller->AppendCompilerPipelineCache().CompilerOptionsCache = CompilerOptions;

        // It's okay for postProcessArgs to be nullptr, but if it's there, it has to be valid.
        if (source.postProcessArgs != nullptr && !source.postProcessArgs->IsValid()) {
            ThrowErrorMessage("Post-process shader arguments is invalid");
            return {};
        }

        ShaderCompileResult CompileResult;
        backend->PostProcessShader(options,source.postProcessArgs.get(), InitialCompileResult, CompileResult);
        // Cache compile result
        controller->AppendCompilerPipelineCache().CompileResultCache = CompileResult;

        return CompileResult;
    }

    SPIRVReflection CompilerPipeline::Reflection() {
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
        const LocalCompilerPipelineCache* cache = controller->GetCompilerPipelineCache();
        
        // Get backend
        ShaderCompilerBackend* backend = controller->GetCompilerContext()->m_Backend.get();
        if (!backend || !backend->IsValid()) {
            ThrowErrorMessage("Shader compiler backend is null");
        }

        // Get shader pipeline generation mode
        ShaderPipelineGenerationMode pipelineMode = backend->GetShaderPipelineGenerationMode();
        // SPIRV compile result
        ShaderCompileResult ReflectionGeneratedSource;
        
        // SPIRV compile lambda
        auto SPIRVCompileLambda = [cache, backend]() -> ShaderCompileResult {
            std::string sourceDescription = cache->SourceCache.sourceDescription;
            ShaderCompileOptionInternal CompilerOptions = cache->CompilerOptionsCache;
            std::string SPIRVEnvironment = backend->SPIRVCompileEnvironment();
            // Set SPIR-V target environment
            CompilerOptions.SPIR_V_TargetEnv = SPIRVEnvironment.c_str();
            CompilerOptions.targetCompilerMode = "-spirv";

            switch (cache->SourceCache.sourceType) {
                case ShaderCompileSource::SourceType::SourcePath:
                    return Internal::GetCompilerInstance()->CompileFromFile(sourceDescription, CompilerOptions);
                    break;
                case ShaderCompileSource::SourceType::SourceCode:
                    return Internal::GetCompilerInstance()->CompileFromString(sourceDescription, CompilerOptions);
                    break;
                default: ThrowErrorMessage("Not implemented");
                    return {};
                    break;
            }
        };
        // Select a mode to generate SPIRV reflection source
        switch (pipelineMode.generationMode) {
            case ShaderPipelineGenerationMode::ReflectionGenerationMode::Use_CompileResultCache:
                ReflectionGeneratedSource = cache->CompileResultCache;
                break;
            case ShaderPipelineGenerationMode::ReflectionGenerationMode::Use_InitialCompileCache:
                ReflectionGeneratedSource = cache->InitialCompileCache;
                break;
            case ShaderPipelineGenerationMode::ReflectionGenerationMode::Use_SourceCacheCompile:
                ReflectionGeneratedSource = SPIRVCompileLambda();
                break;
            default: ThrowErrorMessage("Not implemented");
                return {};
                break;
        }

        // Check compile result
        if(!ReflectionGeneratedSource.success){
            Core::ErrorCapture::Capture(ReflectionGeneratedSource.errorMessage);
            // Default implementation
            struct SPIRVReflection reflection;
            return reflection;
        }

        std::vector<uint32_t> SPIRVByteCode = ConvertUint8ToUint32(ReflectionGeneratedSource.byteCode);
        // Get the reflection generator
        IShaderCompiler* reflectionGenerator = Internal::GetSPIRVReflectionGenerator();
        if (!reflectionGenerator) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("SPIRV reflection generator is null");
#endif
            // Default implementation
            return {};
        }

        // Extract reflection
        SPIRVReflection reflection = reflectionGenerator->ExtractReflection(SPIRVByteCode);
        // Set cache
        controller->AppendCompilerPipelineCache().SPIRVReflectionCache = reflection;
        return reflection;
    }

    CreateShaderDesc CompilerPipeline::CreateShaderDescription() {
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
        const LocalCompilerPipelineCache* cache = controller->GetCompilerPipelineCache();
        // Get backend
        ShaderCompilerBackend* backend = controller->GetCompilerContext()->m_Backend.get();
        if (!backend || !backend->IsValid()) {
            ThrowErrorMessage("Shader compiler backend is null");
        }
        // Get shader pipeline generation mode
        ShaderPipelineGenerationMode pipelineMode = backend->GetShaderPipelineGenerationMode();

        CreateShaderDesc shaderDesc;
        if(pipelineMode.saveMode == ShaderPipelineGenerationMode::ShaderSaveMode::Use_UINT8){
            shaderDesc.byteCode.emplace<std::vector<uint8_t>>(cache->CompileResultCache.byteCode);
        }else if(pipelineMode.saveMode == ShaderPipelineGenerationMode::ShaderSaveMode::Use_UINT32){
            // Convert uint8_t to uint32_t
            const std::vector<uint8_t>& data8 = cache->CompileResultCache.byteCode;
            std::vector<uint32_t> data32;
            const size_t sizeInBytes = data8.size();
            // Check if the size is multiple of sizeof(uint32_t)
            if(sizeInBytes % sizeof(uint32_t) != 0){
#if RHI_ENABLE_DEBUG_INFO
                ThrowErrorMessage("Shader byte code size is not multiple of sizeof(uint32_t)");
#endif
            }
            size_t elementCount = (sizeInBytes + 3) / sizeof(uint32_t);
            data32.resize(elementCount); // Resize to uint32_t size
            std::memcpy(data32.data(), data8.data(), sizeInBytes); // Copy data

            shaderDesc.byteCode.emplace<std::vector<uint32_t>>(data32);
        }else{
            ThrowErrorMessage("Not implemented");
            return {};
        }

        if(cache->CompileResultCache.success == false || cache->CompileResultCache.errorMessage.size() > 0){
            Core::ErrorCapture::Capture( "Create shader description failed: " + cache->CompileResultCache.errorMessage);
            return {};
        }

        return shaderDesc;
    }

}
