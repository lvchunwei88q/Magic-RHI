#pragma once
#include <RHIResourceD3D11.h>
#include <vector>
#include <memory>

//We're going to use a design pattern to consolidate all of the program's resources into this heap.
namespace RHI
{
    /*
    * Descriptor data structure,
    */
    struct DescriptorData
    {
        std::unique_ptr<ConstantBufferViewD3D11> pCBV;
        std::unique_ptr<ShaderResourceViewD3D11> pSRV;
        std::unique_ptr<UnorderedAccessViewD3D11> pUAV;
        std::unique_ptr<RenderTargetViewD3D11> pRTV;
        std::unique_ptr<DepthStencilViewD3D11> pDSV;
        std::unique_ptr<SamplerStateD3D11> pSampler;
        RHIResourceType ViewType;

        void Release()
        {
            pCBV.reset();pSRV.reset();
            pUAV.reset();pRTV.reset();
            pDSV.reset();pSampler.reset();
            ViewType = RHIResourceType::RRT_None;
        }
    };

    class DescriptorHeapD3D11 : public RHIDescriptorHeap
    {
    public:
        DescriptorHeapD3D11(RHIDescriptorHeapType InType, uint32_t InCapacity)
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
        ~DescriptorHeapD3D11() override = default;
        RHIResource* GetDescriptorHeapView(RHIDescriptorHandle handle) const override; // 获取描述符堆视图 直接返回包装引用的资源对象

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

        bool IsFull() const override { return m_FreeList.empty(); }

        // Our DX11 heap by default can store all types of data,
        // but we should standardize using each heap to store the corresponding data.
        void SetDescriptor(RHIDescriptorHandle handle, ConstantBufferViewD3D11* pCBV);
        void SetDescriptor(RHIDescriptorHandle handle, ShaderResourceViewD3D11* pSRV);
        void SetDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewD3D11* pUAV);
        
        void SetDescriptor(RHIDescriptorHandle handle, RenderTargetViewD3D11* pRTV);
        void SetDescriptor(RHIDescriptorHandle handle, DepthStencilViewD3D11* pDSV);
        
        void SetDescriptor(RHIDescriptorHandle handle, SamplerStateD3D11* pSampler);

        const DescriptorData* GetDescriptor(RHIDescriptorHandle handle) const;

    private:
        std::vector<uint32_t> m_FreeList;  // Free index list
        std::vector<DescriptorData> m_Descriptors;  // this is the actual descriptor data
    };
}
