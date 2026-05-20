#pragma once

#include "Common/RHI_API.h"
#include "Common/RHIResourceType.h"
#include <Tools/Cast.h>
#include <string>
#include <memory>
#include <vector>

#ifndef RHI_ENABLE_RESOURCE_INFO // 开启调试信息
    #ifdef _DEBUG
        #define RHI_ENABLE_RESOURCE_INFO 1   // Debug: Run
    #else
        #define RHI_ENABLE_RESOURCE_INFO 0   // Release: Close
    #endif
#endif

namespace RHI
{  
    struct GPUVRamAllocation
    {
        GPUVRamAllocation() = default;
        GPUVRamAllocation(uint64_t InAllocationStart, uint64_t InAllocationSize)
            : AllocationStart(InAllocationStart)
            , AllocationSize(InAllocationSize)
        {
        }

        bool IsValid() const { return AllocationSize > 0; }
        
        // in bytes
        uint64_t AllocationStart{};
        // in bytes
        uint64_t AllocationSize{};
    };

    struct RHIResourceInfo
    {
        std::string Name;
        RHIResourceType Type{ RRT_None };
        GPUVRamAllocation VRamAllocation;
        bool IsTransient{ false };
        bool bValid{ true };
        bool bResident{ true };
    };

    /** The base type of RHI resources. */
    class RHI_API RHIResource : public std::enable_shared_from_this<RHIResource>
    {
    public:
        explicit RHIResource(RHIResourceType InType) : Type(InType) {}
        virtual ~RHIResource() = default;

        RHIResource(const RHIResource&) = delete;
        RHIResource& operator=(const RHIResource&) = delete;

        RHIResourceType GetType() const { return Type; }
        virtual void* GetResource() const { return nullptr; }

#if RHI_ENABLE_RESOURCE_INFO
        virtual void GetInfo(RHIResourceInfo& OutInfo) const;
#endif

        // get shared_ptr
        std::shared_ptr<RHIResource> GetSharedPtr()
        {
            return shared_from_this();
        }

    protected:
        RHIResourceType Type;
    };

    struct RHIDescriptorHandle
    {
        RHIDescriptorHandle() = default;
        RHIDescriptorHandle(RHIDescriptorHeapType InType, uint32_t InIndex)
            : Index(InIndex)
            , Type((uint8_t)InType)
        {
        }
        RHIDescriptorHandle(uint8_t InType, uint32_t InIndex)
            : Index(InIndex)
            , Type(InType)
        {
        }

        inline uint32_t                 GetIndex() const { return Index; }
        inline RHIDescriptorHeapType GetType() const { return (RHIDescriptorHeapType)Type; }
        inline uint8_t                  GetRawType() const { return Type; }

        inline bool IsValid() const { return Index != MAX_uint32_t && Type != (uint8_t)RHIDescriptorHeapType::Invalid; }

    private:
        uint32_t    Index{ MAX_uint32_t };
        uint8_t     Type{ (uint8_t)RHIDescriptorHeapType::Invalid };
    };

    //
    // State blocks
    //

    struct SamplerStateDesc // 采样器状态描述符
    {
        SamplerFilter Filter = SamplerFilter::Bilinear;
        SamplerAddressMode AddressU = SamplerAddressMode::Wrap;
        SamplerAddressMode AddressV = SamplerAddressMode::Wrap;
        SamplerAddressMode AddressW = SamplerAddressMode::Wrap;
        float MipLODBias = 0.0f;
        uint32_t MaxAnisotropy = 1;
        SamplerComparisonFunc ComparisonFunc = SamplerComparisonFunc::Never;
        float BorderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float MinLOD = 0.0f;
        float MaxLOD = 3.402823466e+38f;
    };

    class RHI_API RHISamplerState : public RHIResource 
    {
    public:
        RHISamplerState(const RHIDescriptorHandle& InHandle) 
            : RHIResource(RRT_SamplerState), Handle(InHandle) {}
        virtual ~RHISamplerState() = default;
        
        virtual bool IsImmutable() const { return false; }
        virtual RHIDescriptorHandle GetBindlessHandle() const { return Handle; }

        protected:
            RHIDescriptorHandle Handle;
    };

    //
    // Buffers
    //

    struct BufferDesc
    {
        uint64_t SizeInBytes = 0;
        uint32_t Stride = 0;
        void* InitialData = nullptr;
        // Buffer heap type.
        BufferHeapType HeapType = BufferHeapType::Default; // CPU 不可访问 默认
        BufferBindFlag BindFlags = BufferBindFlag::None;
    };

    class RHI_API RHIBuffer : public RHIResource
    {
    public:
        RHIBuffer(const BufferDesc& InDesc, RHIResourceType InType)
            : RHIResource(InType)
            , SizeInBytes(InDesc.SizeInBytes)
            , Stride(InDesc.Stride)
            , HeapType(InDesc.HeapType)
        {
        }
        virtual ~RHIBuffer() = default;

        uint64_t GetSize() const { return SizeInBytes; }
        uint32_t GetStride() const { return Stride; }
        BufferHeapType GetHeapType() const { return HeapType; }

        virtual void* Map() = 0;
        virtual void Unmap() = 0;

    protected:
        uint64_t SizeInBytes;
        uint32_t Stride;
        BufferHeapType HeapType;
    };

    using RHIVertexBuffer = RHIBuffer;
    using RHIIndexBuffer = RHIBuffer;
    using RHIConstantBuffer = RHIBuffer;
    using RHIStorageBuffer = RHIBuffer;

    // 前向声明和占位符类
    class RHI_API RHIVertexShader : public RHIResource
    {
    public:
        RHIVertexShader() : RHIResource(RRT_VertexShader) {}
    };

    class RHI_API RHIPixelShader : public RHIResource
    {
    public:
        RHIPixelShader() : RHIResource(RRT_PixelShader) {}
    };

    class RHI_API RHIGeometryShader : public RHIResource
    {
    public:
        RHIGeometryShader() : RHIResource(RRT_GeometryShader) {}
    };

    class RHI_API RHIHullShader : public RHIResource
    {
    public:
        RHIHullShader() : RHIResource(RRT_None) {}
    };

    class RHI_API RHIDomainShader : public RHIResource
    {
    public:
        RHIDomainShader() : RHIResource(RRT_None) {}
    };

    class RHI_API RHIComputeShader : public RHIResource
    {
    public:
        RHIComputeShader() : RHIResource(RRT_ComputeShader) {}
    };

    class RHI_API RHIRasterizerState : public RHIResource
    {
    public:
        RHIRasterizerState() : RHIResource(RRT_RasterizerState) {}
    };

    class RHI_API RHIBlendState : public RHIResource
    {
    public:
        RHIBlendState() : RHIResource(RRT_BlendState) {}
    };

    class RHI_API RHIDepthStencilState : public RHIResource
    {
    public:
        RHIDepthStencilState() : RHIResource(RRT_DepthStencilState) {}
    };

    class RHI_API RHIShaderResourceView : public RHIResource
    {
    public:
        RHIShaderResourceView() : RHIResource(RRT_ShaderResourceView) {}
    };

    class RHI_API RHIRenderTargetView : public RHIResource
    {
    public:
        RHIRenderTargetView() : RHIResource(RRT_ShaderResourceView) {}
    };

    class RHI_API RHIDepthStencilView : public RHIResource
    {
    public:
        RHIDepthStencilView() : RHIResource(RRT_ShaderResourceView) {}
    };

    //
    // Command lists
    //

    class RHI_API RHICommandList : public RHIResource
    {
    public:
        RHICommandList(RHICmdListType InType)
            : RHIResource(RRT_None), CmdListType(InType) {}
        virtual ~RHICommandList() = default;

        RHICmdListType GetCmdListType() const { return CmdListType; }

        virtual void BeginRecording() = 0;
        virtual void EndRecording() = 0;

        // 输入装配器
        virtual void IASetPrimitiveTopology(RHIPrimitiveTopology topology, uint32_t controlPointCount = 1) = 0;
        virtual void IASetVertexBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers, const uint64_t* pOffsets = nullptr) = 0;
        virtual void IASetIndexBuffer(RHIBuffer* pIndexBuffer, RHIIndexFormat format, uint64_t offset = 0) = 0;

        // 着色器
        virtual void VSSetShader(class RHIVertexShader* pShader) = 0;
        virtual void PSSetShader(class RHIPixelShader* pShader) = 0;
        virtual void GSSetShader(class RHIGeometryShader* pShader) = 0;
        virtual void HSSetShader(class RHIHullShader* pShader) = 0;
        virtual void DSSetShader(class RHIDomainShader* pShader) = 0;
        virtual void CSSetShader(class RHIComputeShader* pShader) = 0;

        // 着色器资源
        virtual void VSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void PSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void GSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void HSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void DSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;
        virtual void CSSetConstantBuffers(uint32_t startSlot, uint32_t numBuffers, RHIBuffer* const* ppBuffers) = 0;

        virtual void VSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void PSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void GSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void HSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void DSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;
        virtual void CSSetShaderResources(uint32_t startSlot, uint32_t numViews, class RHIShaderResourceView* const* ppViews) = 0;

        virtual void VSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void PSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void GSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void HSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void DSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;
        virtual void CSSetSamplers(uint32_t startSlot, uint32_t numSamplers, RHISamplerState* const* ppSamplers) = 0;

        // 光栅器
        virtual void RSSetViewports(uint32_t numViewports, const RHIViewport* pViewports) = 0;
        virtual void RSSetScissorRects(uint32_t numRects, const RHIRect* pRects) = 0;
        virtual void RSSetState(class RHIRasterizerState* pState) = 0;

        // 输出合并器
        virtual void OMSetRenderTargets(uint32_t numRenderTargets, class RHIRenderTargetView* const* ppViews, class RHIDepthStencilView* pDepthStencilView = nullptr) = 0;
        virtual void OMSetBlendState(class RHIBlendState* pState, const float* blendFactor = nullptr, uint32_t sampleMask = 0xFFFFFFFF) = 0;
        virtual void OMSetDepthStencilState(class RHIDepthStencilState* pState, uint32_t stencilRef = 0) = 0;

        // 绘制
        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) = 0;
        virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) = 0;
        virtual void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) = 0;
        virtual void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

        // 清除
        virtual void ClearRenderTargetView(class RHIRenderTargetView* pView, const float* colorRGBA) = 0;
        virtual void ClearDepthStencilView(class RHIDepthStencilView* pView, RHIClearFlags clearFlags, float depth, uint8_t stencil) = 0;

        // 资源操作
        virtual void CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource) = 0;
        virtual void CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes) = 0;

        // 屏障
        virtual void ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers) = 0;

    protected:
        RHICmdListType CmdListType;
    };

    class RHI_API RHICommandQueue : public RHIResource
    {
    public:
        RHICommandQueue(RHICmdListType InType)
            : RHIResource(RRT_None), QueueType(InType) {}
        virtual ~RHICommandQueue() = default;

        RHICmdListType GetQueueType() const { return QueueType; }

        virtual void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) = 0;
        virtual void WaitForIdle() = 0;

        // 同步操作
        virtual uint64_t Signal() = 0;
        virtual bool GetTimestampFrequency(uint64_t* frequency) = 0;
        virtual bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) = 0;
        virtual uint64_t GetCompletedValue() const = 0;

    protected:
        RHICmdListType QueueType;
    };

} // namespace RHI
