#pragma once

#include "Common/RHI_API.h"
#include "Common/RHIConfig.h"
#include "Common/RHITypes.h"
#include "Common/RHIDefinitions.h"
#include <Common/Check.h>
#include <string>
#include <memory>

namespace RHI
{  
    /**
     * @brief Aligns a value up to the specified alignment.
     * @param Value
     * @param Alignment
     * @return
     */
    inline uint64_t AlignUp(uint64_t Value, uint64_t Alignment)
    {
        return (Value + Alignment - 1) & ~(Alignment - 1);
    }

    /**
     * @brief Aligns a value down to the specified alignment.
     * @param Value
     * @param Alignment
     * @return
     */
    inline uint64_t AlignDown(uint64_t Value, uint64_t Alignment)
    {
        return Value & ~(Alignment - 1);
    }

    /**
     * @brief GPU VRAM allocation descriptor.
     */
    struct GPUVRamAllocation 
    {
        /**
         * @brief
         */
        GPUVRamAllocation() = default;
        /**
         * @brief
         * @param InAllocationStart
         * @param InAllocationSize
         */
        GPUVRamAllocation(uint64_t InAllocationStart, uint64_t InAllocationSize)
            : AllocationStart(InAllocationStart)
            , AllocationSize(InAllocationSize)
        {
        }

        /**
         * @brief
         * @return
         */
        bool IsValid() const { return AllocationSize > 0; }
        
        // in bytes
        uint64_t AllocationStart{};
        // in bytes
        uint64_t AllocationSize{};
    };

    /** The base type of RHI resources. */
    class RHI_API RHIResource : public std::enable_shared_from_this<RHIResource>
    {
    public:
        /**
         * @brief
         * @param InType
         */
        explicit RHIResource(RHIResourceType InType) : Type(InType) {}
        /**
         * @brief
         */
        virtual ~RHIResource() = default;

        /**
         * @brief
         * @param other
         */
        RHIResource(const RHIResource& other) = delete;
        /**
         * @brief
         * @param other
         * @return
         */
        RHIResource& operator=(const RHIResource& other) = delete;

        /**
         * @brief
         * @return
         */
        RHIResourceType GetType() const { return Type; }
        // We probably shouldn’t provide an interface to access resources here because this class itself is a resource.
        //virtual void* GetResource() const { return nullptr; } 

        /**
         * @brief get shared_ptr
         * @return
         */
        std::shared_ptr<RHIResource> GetSharedPtr()
        {
            return shared_from_this();
        }

    protected:
        RHIResourceType Type;
    };

    /**
     * @brief
     */
    struct RHIDescriptorHandle
    {
        /**
         * @brief
         */
        RHIDescriptorHandle() = default;
        /**
         * @brief
         * @param InType
         * @param InIndex
         */
        RHIDescriptorHandle(RHIDescriptorHeapType InType, uint32_t InIndex)
            : Index(InIndex)
            , Type((uint8_t)InType)
        {
        }
        /**
         * @brief
         * @param InType
         * @param InIndex
         */
        RHIDescriptorHandle(uint8_t InType, uint32_t InIndex)
            : Index(InIndex)
            , Type(InType)
        {
        }

        /**
         * @brief
         * @return
         */
        inline uint32_t                 GetIndex() const { return Index; }
        /**
         * @brief
         * @return
         */
        inline RHIDescriptorHeapType    GetType() const { return (RHIDescriptorHeapType)Type; }
        /**
         * @brief
         * @return
         */
        inline uint8_t                  GetRawType() const { return Type; }

        /**
         * @brief
         * @return
         */
        inline bool IsValid() const { return Index != MAX_uint32_t && Type != (uint8_t)RHIDescriptorHeapType::Invalid; }

    private:
        uint32_t    Index{ MAX_uint32_t };
        uint8_t     Type{ (uint8_t)RHIDescriptorHeapType::Invalid };
    };

    /**
     * @brief 描述符堆类型与容量。
     */
    class RHI_API RHIDescriptorHeap : public RHIResource
    {
    public:
        /**
         * @brief
         * @param InType
         * @param InCapacity
         */
        RHIDescriptorHeap(RHIDescriptorHeapType InType, uint32_t InCapacity)
            : RHIResource(RRT_None)
            , HeapType(InType)
            , Capacity(InCapacity)
            , CurrentIndex(0)
        {}
        /**
         * @brief
         */
        virtual ~RHIDescriptorHeap() = default;

        /**
         * @brief
         * @return
         */
        RHIDescriptorHeapType GetHeapType() const { return HeapType; }
        /**
         * @brief
         * @return
         */
        uint32_t GetCapacity() const { return Capacity; }
        /**
         * @brief
         * @return
         */
        uint32_t GetCurrentIndex() const { return CurrentIndex; }
        /**
         * @brief 获取描述符堆视图，直接返回包装引用的资源对象。
         * @param handle
         * @return
         */
        virtual RHIResource* GetDescriptorHeapView(RHIDescriptorHandle handle) const = 0;

        /**
         * @brief
         * @return
         */
        [[nodiscard]] virtual RHIDescriptorHandle Allocate() = 0;
        /**
         * @brief
         * @param handle
         */
        virtual void Free(RHIDescriptorHandle handle) = 0;
        /**
         * @brief
         * @return
         */
        virtual bool IsFull() const { return CurrentIndex >= Capacity; }

    protected:
        RHIDescriptorHeapType HeapType;
        uint32_t Capacity;
        uint32_t CurrentIndex;
    };

    //
    // State blocks
    //

    /**
     * @brief Sampler state descriptor.
     */
    struct SamplerStateDesc
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
        /**
         * @brief
         * @param InHandle
         */
        RHISamplerState(const RHIDescriptorHandle& InHandle) 
            : RHIResource(RRT_SamplerState), Handle(InHandle) {}
        /**
         * @brief
         */
        virtual ~RHISamplerState() = default;
        
        /**
         * @brief
         * @return
         */
        virtual bool IsImmutable() const { return false; }
        /**
         * @brief
         * @return
         */
        virtual RHIDescriptorHandle GetBindlessHandle() const { return Handle; }

        protected:
            RHIDescriptorHandle Handle;
    };

    //
    // Buffers
    //

    /**
     * @brief
     */
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
        /**
         * @brief
         * @param InDesc
         * @param InType
         */
        RHIBuffer(const BufferDesc& InDesc, RHIResourceType InType)
            : RHIResource(InType)
            , SizeInBytes(InDesc.SizeInBytes)
            , Stride(InDesc.Stride)
            , HeapType(InDesc.HeapType)
        {
        }
        /**
         * @brief
         */
        virtual ~RHIBuffer() = default;

        /**
         * @brief
         * @return
         */
        virtual uint64_t GetSize() const { return SizeInBytes; }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetStride() const { return Stride; }   
        /**
         * @brief
         * @return
         */
        virtual BufferHeapType GetHeapType() const { return HeapType; }

        /**
         * @brief
         * @param InHandle
         */
        virtual void SetBindlessHandle(RHIDescriptorHandle InHandle) { Handle = InHandle; }
        /**
         * @brief
         * @return
         */
        virtual RHIDescriptorHandle GetBindlessHandle() const { return Handle; }
        /**
         * @brief
         * @return
         */
        virtual bool HasDescriptorHandle() const { return Handle.IsValid(); }

        /**
         * @brief
         * @return
         */
        virtual void* Map() = 0;
        /**
         * @brief
         */
        virtual void Unmap() = 0;

    protected:
        uint64_t SizeInBytes;
        RHIDescriptorHandle Handle;
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
        /**
         * @brief
         */
        RHIVertexShader() : RHIResource(RRT_VertexShader) {}
        /**
         * @brief
         */
        virtual ~RHIVertexShader() = default;
    };

    class RHI_API RHIPixelShader : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIPixelShader() : RHIResource(RRT_PixelShader) {}
        /**
         * @brief
         */
        virtual ~RHIPixelShader() = default;
    };

    class RHI_API RHIGeometryShader : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIGeometryShader() : RHIResource(RRT_GeometryShader) {}
        /**
         * @brief
         */
        virtual ~RHIGeometryShader() = default;
    };

    class RHI_API RHIHullShader : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIHullShader() : RHIResource(RRT_None) {}
        /**
         * @brief
         */
        virtual ~RHIHullShader() = default;
    };

    class RHI_API RHIDomainShader : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIDomainShader() : RHIResource(RRT_None) {}
        /**
         * @brief
         */
        virtual ~RHIDomainShader() = default;
    };

    class RHI_API RHIComputeShader : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIComputeShader() : RHIResource(RRT_ComputeShader) {}
        /**
         * @brief
         */
        virtual ~RHIComputeShader() = default;
    };

    class RHI_API RHIRasterizerState : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIRasterizerState() : RHIResource(RRT_RasterizerState) {}
    };

    class RHI_API RHIBlendState : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIBlendState() : RHIResource(RRT_BlendState) {}
    };

    class RHI_API RHIDepthStencilState : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIDepthStencilState() : RHIResource(RRT_DepthStencilState) {}
    };

    class RHI_API RHITexture : public RHIResource
    {
    public:
        /**
         * @brief
         * @param InDesc
         */
        RHITexture(const TextureDesc& InDesc) : RHIResource(RRT_Texture), m_Desc(InDesc) {}
        /**
         * @brief
         */
        RHITexture() : RHIResource(RRT_Texture) {}
        /**
         * @brief
         */
        virtual ~RHITexture() = default;

        /**
         * @brief
         * @return
         */
        virtual uint64_t GetSize()              const = 0;
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetWidth()             const { return m_Desc.Width;        }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetHeight()            const { return m_Desc.Height;       }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetDepth()             const { return m_Desc.Depth;        }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetMipLevels()         const { return m_Desc.MipLevels;    }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetArrayLayers()       const { return m_Desc.ArrayLayers;  }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetSampleCount()       const { return m_Desc.SampleCount;  }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetSampleQuality()     const { return m_Desc.SampleQuality;}
        /**
         * @brief
         * @return
         */
        virtual RHITextureFormat GetFormat()    const { return m_Desc.Format;       }
        /**
         * @brief
         * @return
         */
        virtual uint32_t GetFlags()             const { return m_Desc.Flags;        }
        /**
         * @brief
         * @return
         */
        virtual TextureDesc GetDesc()           const { return m_Desc;              }

    private:
        TextureDesc m_Desc;
    };

    class RHI_API RHIConstantBufferView : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIConstantBufferView() : RHIResource(RRT_ConstantBufferView) {}

        /**
         * @brief
         * @return
         */
        virtual uint64_t GetGPUVirtualAddress() const = 0;
    };

    class RHI_API RHIShaderResourceView : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIShaderResourceView() : RHIResource(RRT_ShaderResourceView) {}

        /**
         * @brief
         * @return
         */
        virtual uint64_t GetGPUVirtualAddress() const = 0;
    };

    class RHI_API RHIUnorderedAccessView : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIUnorderedAccessView() : RHIResource(RRT_UnorderedAccessView) {}

        /**
         * @brief
         * @return
         */
        virtual uint64_t GetGPUVirtualAddress() const = 0;
    };

    class RHI_API RHIRenderTargetView : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIRenderTargetView() : RHIResource(RRT_RenderTargetView) {}
    };

    class RHI_API RHIDepthStencilView : public RHIResource
    {
    public:
        /**
         * @brief
         */
        RHIDepthStencilView() : RHIResource(RRT_DepthStencilView) {}
    };
} // namespace RHI
