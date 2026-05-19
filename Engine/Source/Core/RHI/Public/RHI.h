#pragma once

#include "Common/RHI_API.h"

#include <memory>
#include <string>

#ifndef RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN
#define RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN 1
#endif

#ifndef RHI_DESCRIPTOR_HEAP_SIZE_STANDARD
#define RHI_DESCRIPTOR_HEAP_SIZE_STANDARD 1024
#endif

#ifndef RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER
#define RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER 256
#endif

#ifndef RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET
#define RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET 32
#endif

#ifndef RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL
#define RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL 32
#endif

#ifndef RHI_MULTI_BUFFERING
#define RHI_MULTI_BUFFERING 2
#endif

namespace RHI
{
    /*
     * 使用前向声明时尤其对于C++智能指针来说我们应该将构造与构析函数放置在有完整类型的文件中，否则将会出现类型不匹配，
     * 因为智能指针需要了解这个Type的内存布局
     */
    // CommandList & CommandQueue Forward Declaration
    class RHICommandList;
    class RHICommandQueue;
    // forward declarations
    class RHISamplerState;
    class RHIBuffer;
    struct SamplerStateDesc;
    struct BufferDesc;
    enum class RHICmdListType : uint8_t;

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

        virtual std::shared_ptr<RHISamplerState> CreateSamplerState(const SamplerStateDesc& desc) = 0;
        virtual void DeleteSamplerState(std::shared_ptr<RHI::RHISamplerState>& samplerState) = 0;

        virtual std::shared_ptr<RHIBuffer> CreateBuffer(const BufferDesc& desc) = 0;
        virtual void DeleteBuffer(std::shared_ptr<RHI::RHIBuffer>& buffer) = 0;

        virtual std::shared_ptr<RHICommandList> CreateCommandList(RHICmdListType type) = 0;
        virtual std::shared_ptr<RHICommandQueue> GetCommandQueue(RHICmdListType Type) const = 0;

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
