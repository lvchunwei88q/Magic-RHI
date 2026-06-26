#include "RHIShaderCompiler.h"
#include "IO.h"

namespace RHI {
    // ========== Compiler Context Controller ==========
    // Get compiler context controller instance
    IShaderCompiler* IRHIModule::GetCompilerContextController(){
        return &CompilerContextController::Get();
    }

    CompilerContextController::CompilerContextController() = default;
    CompilerContextController::~CompilerContextController() = default;

    // Initialize Compiler context instance
    bool CompilerContextController::InitializeCompilerContext() {
        m_Context = std::make_unique<ShaderCompilerContext>();
        bool isInitialized = m_Context->Initialize();
        if (!isInitialized) {
            // init shader compiler failed
            Core::ErrorCapture::Capture("Failed to initialize shader compiler!");
        }
        return m_Context != nullptr && isInitialized;
    }

    void CompilerContextController::ShutdownCompilerContext() {
        // Shutdown DXC compiler
        m_Context.reset();
    }

    // ========== Internal compile core function ==========
    ShaderCompileResult CompileInternal(
        const std::string& hlslSource,
        const LocalShaderCompileOption& options,
        const ShaderCompilerContext& context) {

        ShaderCompileResult result;

        // Check ShaderCompilerContext is initialized
        if (!context.IsInitialized()) {
            result.errorMessage = "Shader compiler context not initialized!";
            return result;
        }

        // Create source Blob
        std::wstring wSource = IO::ToWideString(hlslSource);
        ComPtr<IDxcBlobEncoding> pSourceBlob;
        HRESULT hr = context.utils->CreateBlob(
            wSource.c_str(),
            static_cast<UINT32>(wSource.size() * sizeof(wchar_t)),
            DXC_CP_WIDE,
            &pSourceBlob
        );
        if (FAILED(hr)) {
            result.errorMessage = "Failed to create source blob. HRESULT: " + std::to_string(hr);
            return result;
        }

        // Create DxcBuffer wrapper
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = pSourceBlob->GetBufferPointer();
        sourceBuffer.Size = pSourceBlob->GetBufferSize();
        sourceBuffer.Encoding = DXC_CP_WIDE;

        // Build argument list
        std::vector<std::wstring> m_ArgTempStorage;
        auto args = BuildArguments(options,m_ArgTempStorage);

        // Execute compilation
        ComPtr<IDxcResult> pResult;
        hr = context.compiler->Compile(
            &sourceBuffer,
            args.data(),
            static_cast<UINT32>(args.size()),
            context.includeHandler.Get(),
            IID_PPV_ARGS(&pResult)
        );
        if (FAILED(hr)) {
            result.errorMessage = "Compile() failed. HRESULT: " + std::to_string(hr);
            return result;
        }

        // Get compilation status
        HRESULT status;
        if (FAILED(pResult->GetStatus(&status))) {
            result.errorMessage = "Failed to get compilation status!";
            return result;
        }
        if (FAILED(status)) {
            ComPtr<IDxcBlobEncoding> pErrors;
            if (SUCCEEDED(pResult->GetErrorBuffer(&pErrors)) && pErrors) {
                result.errorMessage = reinterpret_cast<const char*>(pErrors->GetBufferPointer());
            } else {
                result.errorMessage = "Unknown compilation error!";
            }
            return result;
        }

        // Get errors (optional)
        ComPtr<IDxcBlobEncoding> pErrors;
        if (SUCCEEDED(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)) &&
            pErrors && pErrors->GetBufferSize() > 0) {
            result.errorMessage = reinterpret_cast<const char*>(pErrors->GetBufferPointer());
        }

        // Get SPIR-V bytecode
        ComPtr<IDxcBlob> pBytecode;
        if (FAILED(pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pBytecode), nullptr))) {
            result.errorMessage = "Failed to get output bytecode!";
            return result;
        }

        // Copy bytecode to result
        const uint32_t* data = reinterpret_cast<const uint32_t*>(pBytecode->GetBufferPointer());
        size_t wordCount = pBytecode->GetBufferSize() / sizeof(uint32_t);
        result.byteCode.assign(data, data + wordCount);
        result.bytecodeSize = wordCount * sizeof(uint32_t);
        result.success = true;

        return result;
    }

    // ========== Build DXC arguments ==========
    std::vector<const wchar_t*> BuildArguments(
        const LocalShaderCompileOption& options,
        std::vector<std::wstring>& m_ArgStorage) {
        
        std::vector<const wchar_t*> args;
        // Because vector reallocates memory every time we push_back, 
        // we just record the index of each parameter, and at the end we give all the parameter pointers to args.
        std::vector<size_t> indexs;

        auto addArg = [&](const std::string& str) {
            // Record the index of the next parameter
            indexs.push_back(args.size());
            m_ArgStorage.push_back(IO::ToWideString(str));
            // At this point, we're just declaring a placeholder
            args.push_back(nullptr);  
        };

        // Base arguments
        addArg(options.targetCompilerMode);  // Output User Selected Compiler mode
        args.push_back(L"-E");
        addArg(options.entryPoint);
        args.push_back(L"-T");
        addArg(options.targetProfile);

        // Optimization level
        if (options.optimize) {
            args.push_back(L"-O3");
        } else {
            args.push_back(L"-O0");
        }

        // Enable debug information for the shader
        if (options.debugInfo) {
            // DXC_ARG_DEBUG = "-Zi" : Generate debug info for PDB (DXIL debug)
            args.push_back(DXC_ARG_DEBUG);
            
            // Embed source file name, line numbers and column numbers into SPIR-V
            // Allows RenderDoc/NSight to display HLSL source instead of disassembly
            args.push_back(L"-fspv-debug=vulkan-with-source");
            
            // Enable SPV_KHR_non_semantic_info extension
            // Allows debug metadata to be embedded without affecting shader execution
            // (GPU drivers ignore these, debugging tools read them)
            args.push_back(L"-fspv-extension=SPV_KHR_non_semantic_info");
        }

        // HLSL version
        args.push_back(L"-HV");
        addArg(options.DEFAULT_HLSL_VERSION);

        // SPIR-V target environment
        std::string targetEnv = options.targetEnv;
        if (!targetEnv.empty()) {
            std::string envArg = "-fspv-target-env=" + targetEnv;  // ← append targetEnv to "-fspv-target-env="
            addArg(envArg);  // add "-fspv-target-env=vulkan1.3"
        }

        // 16-bit types enabled
        if (options.enable16BitTypes) {
            args.push_back(L"-enable-16bit-types");
        }

        // Matrix packing enabled
        if (options.enableMatrixPacking) {
            args.push_back(L"-fspv-matrix-packing");
        }

        // Macro definitions
        for (const auto& macro : options.Macros) {
            std::string def = macro.name + "=" + macro.definition;
            args.push_back(L"-D");
            addArg(def);
        }

        // Include paths
        for (const auto& path : options.includePaths) {
            args.push_back(L"-I");
            addArg(path);
        }

        // Other common flags
#if RHI_ENABLE_DEBUG_INFO
        // warnings in debug mode is error
        args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);       // Warning as error
#else
        // print warnings in release
#endif

        // Fill in the parameter pointers to args
        for (size_t i = 0; i < indexs.size(); i++)
        {
            size_t arg_index = indexs[i];
            const std::wstring& Arg = m_ArgStorage[i];
            args[arg_index] = Arg.c_str();
        }

        return args;
    }
}
