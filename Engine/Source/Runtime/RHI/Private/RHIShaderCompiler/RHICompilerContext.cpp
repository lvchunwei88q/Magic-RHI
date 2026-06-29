#include "Common/Check.h"
#include "RHIShaderCompiler.h"
#include "IO.h"

namespace RHI {
    // ========== Compiler Context Controller ==========
    // Get compiler context controller instance
    std::unique_ptr<IShaderCompiler> IRHIModule::GetCompilerContextController(){
        return std::make_unique<CompilerContextController>();
    }

    CompilerContextController::CompilerContextController() = default;
    CompilerContextController::~CompilerContextController() {
        // if compiler context is not shutdown, shutdown it
        if (m_ThreadContext_State != CompilerContextState::Shutdown) {
#if RHI_ENABLE_DEBUG_INFO
            ThrowErrorMessage("Shader compiler context is not shutdown!");
#endif
            ShutdownCompilerThreadContext();
        }
    };

    // Initialize Compiler context instance
    bool CompilerContextController::InitializeCompilerThreadContext() {
        // Initialize cache
        m_Cache = std::make_unique<LocalCompilerPipelineCache>();
        // Initialize shader context
        m_Context = std::make_unique<LocalShaderCompilerContext>();
        bool isInitialized = m_Context->Initialize();
        if (!isInitialized) {
            // init shader compiler failed
            Core::ErrorCapture::Capture("Failed to initialize shader compiler!");
        }

        // Set compiler context to this controller
        SetCompilerContext(this);
        // Return if initialized and state to initialized
        m_ThreadContext_State = CompilerContextState::Initialized;
        return m_Context != nullptr && isInitialized;
    }

    void CompilerContextController::ShutdownCompilerThreadContext() {
        // Shutdown DXC compiler
        m_Context.reset();
        // Reset cache
        m_Cache.reset();
        // Set compiler context to nullptr
        SetCompilerContext(nullptr);

        // Set state to shutdown
        m_ThreadContext_State = CompilerContextState::Shutdown;
    }
}
