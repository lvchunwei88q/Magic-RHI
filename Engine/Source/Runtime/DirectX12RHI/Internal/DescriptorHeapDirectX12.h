#pragma once
#include <RHIResourceDirectX12.h>
#include <d3d12.h>
#include <vector>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    struct DescriptorData
    {
        std::unique_ptr<ConstantBufferViewDirectX12> pCBV;
        std::unique_ptr<ShaderResourceViewDirectX12> pSRV;
        std::unique_ptr<UnorderedAccessViewDirectX12> pUAV;
        std::unique_ptr<RenderTargetViewDirectX12> pRTV;
        std::unique_ptr<DepthStencilViewDirectX12> pDSV;
        std::unique_ptr<SamplerStateDirectX12> pSampler;
        RHIResourceType ViewType;

        void Release(){
            pCBV.reset();pSRV.reset();
            pUAV.reset();pRTV.reset();
            pDSV.reset();pSampler.reset();
            ViewType = RHIResourceType::RRT_None;
        }
    };

    class DescriptorHeapDirectX12 : public RHIDescriptorHeap
    {
    public:
        // 构造函数
        // @param pHeap: DirectX 12 描述符堆
        // @param InType: 描述符堆类型
        // @param InCapacity: 描述符堆容量(描述符数量)
        // @param InDescriptorSize: 每个描述符的大小
        DescriptorHeapDirectX12(ID3D12DescriptorHeap* pHeap, RHIDescriptorHeapType InType, uint32_t InCapacity, uint32_t InDescriptorSize)
            : RHIDescriptorHeap(InType, InCapacity)
            , m_pHeap(pHeap)
            , m_DescriptorSize(InDescriptorSize)
        {
            m_FreeList.reserve(InCapacity); // 预分配空闲索引容器的内存
            m_Descriptors.reserve(InCapacity); // 预分配描述符容器的内存
            for (uint32_t i = 0; i < InCapacity; ++i){
                m_Descriptors.push_back({});
                m_FreeList.push_back(i);
            }
        }
        ~DescriptorHeapDirectX12() override = default;

        ID3D12DescriptorHeap* GetHeap() const { return m_pHeap.Get(); }
        uint32_t GetDescriptorSize() const { return m_DescriptorSize; }
        RHIResource* GetDescriptorHeepView(RHIDescriptorHandle handle) const override; // 获取描述符堆视图 直接返回包装引用的资源对象

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

        void SetDescriptor(RHIDescriptorHandle handle, ConstantBufferViewDirectX12* pCBV);
        void SetDescriptor(RHIDescriptorHandle handle, ShaderResourceViewDirectX12* pSRV);
        void SetDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewDirectX12* pUAV);
        
        void SetDescriptor(RHIDescriptorHandle handle, RenderTargetViewDirectX12* pRTV);
        void SetDescriptor(RHIDescriptorHandle handle, DepthStencilViewDirectX12* pDSV);
        
        void SetDescriptor(RHIDescriptorHandle handle, SamplerStateDirectX12* pSampler);

        const DescriptorData* GetDescriptor(RHIDescriptorHandle handle) const;

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

    private:
        ComPtr<ID3D12DescriptorHeap> m_pHeap;
        uint32_t m_DescriptorSize;

        std::vector<uint32_t> m_FreeList;  // 空闲的索引
        std::vector<DescriptorData> m_Descriptors;  // 存储实际的描述符数据
    };
}
