#pragma once
#include <RHIResourceDirectX11.h>
#include <vector>
#include <memory>

namespace RHI
{
    /*
    * 描述符数据
    */
    struct DescriptorData
    {
        std::unique_ptr<ConstantBufferViewDirectX11> pCBV;
        std::unique_ptr<ShaderResourceViewDirectX11> pSRV;
        std::unique_ptr<UnorderedAccessViewDirectX11> pUAV;
        std::unique_ptr<RenderTargetViewDirectX11> pRTV;
        std::unique_ptr<DepthStencilViewDirectX11> pDSV;
        std::unique_ptr<SamplerStateDirectX11> pSampler;

        void Release()
        {
            pCBV.reset();pSRV.reset();
            pUAV.reset();pRTV.reset();
            pDSV.reset();pSampler.reset();
        }
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

        void SetDescriptor(RHIDescriptorHandle handle, ConstantBufferViewDirectX11* pCBV);
        void SetDescriptor(RHIDescriptorHandle handle, ShaderResourceViewDirectX11* pSRV);
        void SetDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewDirectX11* pUAV);
        void SetDescriptor(RHIDescriptorHandle handle, RenderTargetViewDirectX11* pRTV);
        void SetDescriptor(RHIDescriptorHandle handle, DepthStencilViewDirectX11* pDSV);
        void SetDescriptor(RHIDescriptorHandle handle, SamplerStateDirectX11* pSampler);

        const DescriptorData* GetDescriptor(RHIDescriptorHandle handle) const;

    private:
        std::vector<uint32_t> m_FreeList;  // 空闲的索引
        std::vector<DescriptorData> m_Descriptors;  // 存储实际的描述符数据
    };
}
