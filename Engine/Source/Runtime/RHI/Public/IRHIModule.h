// ========== RHI/Public/IRHILoader.h ==========

#pragma once
#include <memory>
#include "Common/RHI_API.h"
#include "Common/RHIPlatformDetection.h"

// Forward Declaration
class Device;
class SwapChain;

namespace RHI
{
    // RHI Loader Interface
    class IRHILoader;

    // RHI Module Interface
    class RHI_API IRHIModule
    {
    public:
        virtual ~IRHIModule() = default;

        static IRHILoader* GetRHILoader();
    };

    class RHI_API IRHILoader
    {
    public:
        virtual ~IRHILoader() = default;

        virtual bool Load(RHIType type) = 0;
        virtual void Unload() = 0;
        
        virtual std::unique_ptr<Device> CreateDevice() = 0;
        virtual std::unique_ptr<SwapChain> CreateSwapChain() = 0;
        
        virtual bool IsLoaded() const = 0;
        virtual RHIType GetRHIType() const = 0;
    };
} // namespace RHI