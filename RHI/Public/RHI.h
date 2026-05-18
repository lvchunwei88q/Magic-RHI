#pragma once

#include "Common/RHI_API.h"

#include <memory>
#include <string>

#ifndef RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN
#define RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN 1
#endif

namespace RHI
{
    enum class RHIType
    {
        DirectX11,
        DirectX12
    };

    struct SwapChainDesc
    {
        void* WindowHandle;
        uint32_t Width;
        uint32_t Height;
        uint32_t BufferCount = 2;
        bool VSync = false;
    };

    class RHI_API Device
    {
    public:
        virtual ~Device() = default;

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;

        virtual bool IsValid() const = 0;
        virtual RHIType GetType() const = 0;
        virtual const std::wstring& GetAdapterName() const = 0;

        static std::unique_ptr<Device> Create(RHIType type);
    };

    class RHI_API SwapChain
    {
    public:
        virtual ~SwapChain() = default;

        virtual bool Initialize(Device* device, const SwapChainDesc& desc) = 0;
        virtual void Shutdown() = 0;

        virtual bool IsValid() const = 0;
        virtual void Present() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        static std::unique_ptr<SwapChain> Create(RHIType type);
    };
}
