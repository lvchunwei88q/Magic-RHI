#pragma once

#include "Common/RHI_API.h"
#include "Common/RHIResourceType.h"
#include <Tools/Cast.h>
#include <string>
#include <memory>

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
        //virtual void* GetResource() const { return nullptr; } 这里不应该获取到资源我们要对用户隐藏起来

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
        virtual ~RHIVertexShader() = default;
    };

    class RHI_API RHIPixelShader : public RHIResource
    {
    public:
        RHIPixelShader() : RHIResource(RRT_PixelShader) {}
        virtual ~RHIPixelShader() = default;
    };

    class RHI_API RHIGeometryShader : public RHIResource
    {
    public:
        RHIGeometryShader() : RHIResource(RRT_GeometryShader) {}
        virtual ~RHIGeometryShader() = default;
    };

    class RHI_API RHIHullShader : public RHIResource
    {
    public:
        RHIHullShader() : RHIResource(RRT_None) {}
        virtual ~RHIHullShader() = default;
    };

    class RHI_API RHIDomainShader : public RHIResource
    {
    public:
        RHIDomainShader() : RHIResource(RRT_None) {}
        virtual ~RHIDomainShader() = default;
    };

    class RHI_API RHIComputeShader : public RHIResource
    {
    public:
        RHIComputeShader() : RHIResource(RRT_ComputeShader) {}
        virtual ~RHIComputeShader() = default;
    };

    struct ShaderCompileDesc
    {
        ShaderType Type;
        const char* SourceCode = nullptr;
        const char* EntryPoint = "main";
        const char* Profile = nullptr; // 对于普通的Shader而言我们不希望你去指定Profile，因为在不同的平台上的Shader Model的支持程度是不同的，我们的Shader编译器会根据平台自动选择合适的Profile
        const char* FilePath = nullptr;
        bool EnableDebugInfo = false;
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

} // namespace RHI
