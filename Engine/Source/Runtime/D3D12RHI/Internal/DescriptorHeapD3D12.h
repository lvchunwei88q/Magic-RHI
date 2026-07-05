#pragma once
#include <RHIResourceD3D12.h>
#include <d3d12.h>
#include <vector>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    struct DescriptorData
    {
        std::unique_ptr<ConstantBufferViewD3D12> pCBV;
        std::unique_ptr<ShaderResourceViewD3D12> pSRV;
        std::unique_ptr<UnorderedAccessViewD3D12> pUAV;
        std::unique_ptr<RenderTargetViewD3D12> pRTV;
        std::unique_ptr<DepthStencilViewD3D12> pDSV;
        std::unique_ptr<SamplerStateD3D12> pSampler;
        RHIResourceType ViewType;

        void Release(){
            pCBV.reset();pSRV.reset();
            pUAV.reset();pRTV.reset();
            pDSV.reset();pSampler.reset();
            ViewType = RHIResourceType::RRT_None;
        }
    };

    class DescriptorHeapD3D12 : public RHIDescriptorHeap
    {
    public:
        // 构造函数
        // @param pHeap: DirectX 12 描述符堆
        // @param InType: 描述符堆类型
        // @param InCapacity: 描述符堆容量(描述符数量)
        // @param InDescriptorSize: 每个描述符的大小
        DescriptorHeapD3D12(ID3D12DescriptorHeap* pHeap, RHIDescriptorHeapType InType, uint32_t InCapacity, uint32_t InDescriptorSize)
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
        ~DescriptorHeapD3D12() override = default;

        ID3D12DescriptorHeap* GetHeap() const { return m_pHeap.Get(); }
        uint32_t GetDescriptorSize() const { return m_DescriptorSize; }
        RHIResource* GetDescriptorHeapView(RHIDescriptorHandle handle) const override; // 获取描述符堆视图 直接返回包装引用的资源对象

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

        // Set descriptor view
        void SetCBVDescriptor(RHIDescriptorHandle handle, ConstantBufferViewD3D12* pCBV);
        void SetSRVDescriptor(RHIDescriptorHandle handle, ShaderResourceViewD3D12* pSRV);
        void SetUAVDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewD3D12* pUAV);
        void SetRTVDescriptor(RHIDescriptorHandle handle, RenderTargetViewD3D12* pRTV);
        void SetDSVDescriptor(RHIDescriptorHandle handle, DepthStencilViewD3D12* pDSV);
        void SetSamplerDescriptor(RHIDescriptorHandle handle, SamplerStateD3D12* pSampler);

        const DescriptorData* GetAnyDescriptor(RHIDescriptorHandle handle) const;

        [[nodiscard]] RHIDescriptorHandle Allocate() override;
        void Free(RHIDescriptorHandle handle) override;

    private:
        ComPtr<ID3D12DescriptorHeap> m_pHeap;
        uint32_t m_DescriptorSize;

        std::vector<uint32_t> m_FreeList;  // 空闲的索引
        std::vector<DescriptorData> m_Descriptors;  // 存储实际的描述符数据
    };
}
