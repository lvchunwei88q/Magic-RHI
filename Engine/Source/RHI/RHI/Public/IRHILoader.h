// ========== RHI/Public/IRHILoader.h ==========

#pragma once
#include <memory>
#include "Common/RHI_API.h"
#include "Common/RHIPlatformDetection.h"

// 需要预先包含 Device 和 SwapChain 类，否则会报错
class Device;
class SwapChain;
struct SwapChainDesc;

namespace RHI
{

    // RHI Loader Interface
    class RHI_API IRHILoader
    {
    public:
        virtual ~IRHILoader() = default;

        virtual bool Load(RHIType type) = 0;
        virtual void Unload() = 0;
        
        virtual std::unique_ptr<Device> CreateDevice() = 0;
        virtual std::unique_ptr<SwapChain> CreateSwapChain(Device* device, const SwapChainDesc& desc) = 0;
        
        virtual bool IsLoaded() const = 0;
        virtual RHIType GetLoadedRHIType() const = 0;
    };

    RHI_API IRHILoader* GetLoader();

} // namespace RHI