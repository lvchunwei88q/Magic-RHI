#pragma once
#include <RHIResourceDirectX11.h>
#include <vector>

namespace RHI
{
    /*
    * 描述符数据
    */
    union DescriptorData
    {
        BufferDirectX11* pBuffer;
        ShaderResourceViewDirectX11* pSRV;
        UnorderedAccessViewDirectX11* pUAV;
        RenderTargetViewDirectX11* pRTV;
        DepthStencilViewDirectX11* pDSV;
        SamplerStateDirectX11* pSampler;
        // 通用指针
        void* pGeneric = nullptr;
    };

    class DescriptorHeapDirectX11 : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapDirectX11(RHIDescriptorHeapType InType, uint32_t InCapacity)
            : RHIDescriptorHeap(InType, InCapacity)
        {
            m_FreeList.reserve(InCapacity);
            m_Descriptors.reserve(InCapacity);
            for (uint32_t i = 0; i < InCapacity; ++i)
            {
                m_Descriptors.push_back({nullptr});
                m_FreeList.push_back(i);
            }
        }
        ~DescriptorHeapDirectX11() override = default;

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

        bool IsFull() const override { return m_FreeList.empty(); }

        void SetDescriptor(RHIDescriptorHandle handle, BufferDirectX11* pBuffer);
        void SetDescriptor(RHIDescriptorHandle handle, ShaderResourceViewDirectX11* pSRV);
        void SetDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewDirectX11* pUAV);
        void SetDescriptor(RHIDescriptorHandle handle, RenderTargetViewDirectX11* pRTV);
        void SetDescriptor(RHIDescriptorHandle handle, DepthStencilViewDirectX11* pDSV);
        void SetDescriptor(RHIDescriptorHandle handle, SamplerStateDirectX11* pSampler);

        DescriptorData GetDescriptor(RHIDescriptorHandle handle) const;

    private:
        std::vector<uint32_t> m_FreeList;  // 空闲的索引
        std::vector<DescriptorData> m_Descriptors;  // 存储实际的描述符数据
    };
}
