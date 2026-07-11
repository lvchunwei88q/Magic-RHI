#include <Common/Check.h>
#include "DescriptorHeapD3D12.h"
#include "RHID3D12.h"

#define IMPLEMENT_SET_DESCRIPTOR(Type, Member, View_Type, ErrorFunc)             \
        if (!handle.IsValid() || handle.GetType() != HeapType)                  \
            return;                                                             \
        uint32_t index = handle.GetIndex();                                     \
        if (index < Capacity)                                                   \
        {                                                                       \
            m_Descriptors[index].Member = std::unique_ptr<Type>(Member);        \
            m_Descriptors[index].ViewType = View_Type;                          \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            auto error = ErrorFunc;                                             \
            error();                                                            \
        }

namespace RHI
{
    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapD3D12::GetCPUHandle(uint32_t index) const
    {
        if (!m_pHeap || index >= Capacity)
        {
            return {};
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_DescriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapD3D12::GetGPUHandle(uint32_t index) const
    {
        if (!m_pHeap || index >= Capacity)
        {
            return {};
        }

        D3D12_GPU_DESCRIPTOR_HANDLE handle = m_pHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += index * m_DescriptorSize;
        return handle;
    }
    
    RHIResource* DescriptorHeapD3D12::GetDescriptorHeapView(RHIDescriptorHandle handle) const
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            ThrowErrorMessage("Error GetDescriptorHeapView Unknown index range");
            return nullptr;
        }
        uint32_t index = handle.GetIndex();
        auto& descriptor = m_Descriptors[index];
        if (descriptor.ViewType == RHIResourceType::RRT_None)
        {
            ThrowErrorMessage("Error GetDescriptorHeapView Unknown index range");
            return nullptr;
        }
        return descriptor.ViewType == RHIResourceType::RRT_ConstantBufferView
            ? SafeCast<RHIResource>(descriptor.pCBV.get())
            : descriptor.ViewType == RHIResourceType::RRT_ShaderResourceView
            ? SafeCast<RHIResource>(descriptor.pSRV.get())
            : descriptor.ViewType == RHIResourceType::RRT_UnorderedAccessView
            ? SafeCast<RHIResource>(descriptor.pUAV.get())
            : descriptor.ViewType == RHIResourceType::RRT_RenderTargetView
            ? SafeCast<RHIResource>(descriptor.pRTV.get())
            : descriptor.ViewType == RHIResourceType::RRT_DepthStencilView
            ? SafeCast<RHIResource>(descriptor.pDSV.get())
            : descriptor.ViewType == RHIResourceType::RRT_SamplerState
            ? SafeCast<RHIResource>(descriptor.pSampler.get())
            : nullptr;
    }
    
    RHIDescriptorHandle DescriptorHeapD3D12::Allocate()
    {
        if (IsFull())
        {
            ThrowErrorMessage("Descriptor heap is full");
            return RHIDescriptorHandle();
        }

        uint32_t index = m_FreeList.back();
        m_FreeList.pop_back();
        
        // Update current index
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
        
        return RHIDescriptorHandle(HeapType, index);
    }

    void DescriptorHeapD3D12::Free(RHIDescriptorHandle handle)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return;
        }
        
        uint32_t index = handle.GetIndex();
        
        // Validate index range
        if (index >= Capacity)
        {
            ThrowErrorMessage("Error Unknown index range");
            return;
        }
        
        // Prevent double free
#if _DEBUG
        auto it = std::find(m_FreeList.begin(), m_FreeList.end(), index);
        if (it != m_FreeList.end())
        {
            // If already free, double free detected
            ThrowErrorMessage("Double free detected");
            return;
        }
#endif
        
        m_Descriptors[index].Release();
        m_FreeList.push_back(index);
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
    }

    void DescriptorHeapD3D12::SetCBVDescriptor(RHIDescriptorHandle handle, ConstantBufferViewD3D12* pCBV)
    {
        IMPLEMENT_SET_DESCRIPTOR(ConstantBufferViewD3D12, pCBV, RHIResourceType::RRT_ConstantBufferView,[](){
            ThrowErrorMessage("Error SetDescriptor ConstantBufferView Unknown index range");
        });
    }

    void DescriptorHeapD3D12::SetSRVDescriptor(RHIDescriptorHandle handle, ShaderResourceViewD3D12* pSRV)
    {
        IMPLEMENT_SET_DESCRIPTOR(ShaderResourceViewD3D12, pSRV, RHIResourceType::RRT_ShaderResourceView,[](){
            ThrowErrorMessage("Error SetDescriptor ShaderResourceView Unknown index range");
        });
    }

    void DescriptorHeapD3D12::SetUAVDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewD3D12* pUAV)
    {
        IMPLEMENT_SET_DESCRIPTOR(UnorderedAccessViewD3D12, pUAV, RHIResourceType::RRT_UnorderedAccessView,[](){
            ThrowErrorMessage("Error SetDescriptor UnorderedAccessView Unknown index range");
        });
    }

    void DescriptorHeapD3D12::SetRTVDescriptor(RHIDescriptorHandle handle, RenderTargetViewD3D12* pRTV)
    {
        IMPLEMENT_SET_DESCRIPTOR(RenderTargetViewD3D12, pRTV, RHIResourceType::RRT_RenderTargetView,[](){
            ThrowErrorMessage("Error SetDescriptor RenderTargetView Unknown index range");
        });
    }

    void DescriptorHeapD3D12::SetDSVDescriptor(RHIDescriptorHandle handle, DepthStencilViewD3D12* pDSV)
    {
        IMPLEMENT_SET_DESCRIPTOR(DepthStencilViewD3D12, pDSV, RHIResourceType::RRT_DepthStencilView,[](){
            ThrowErrorMessage("Error SetDescriptor DepthStencilView Unknown index range");
        });
    }

    void DescriptorHeapD3D12::SetSamplerDescriptor(RHIDescriptorHandle handle, SamplerStateD3D12* pSampler)
    {
        IMPLEMENT_SET_DESCRIPTOR(SamplerStateD3D12, pSampler, RHIResourceType::RRT_SamplerState,[](){
            ThrowErrorMessage("Error SetDescriptor SamplerState Unknown index range");
        });
    }

    const DescriptorData* DescriptorHeapD3D12::GetAnyDescriptor(RHIDescriptorHandle handle) const
    {   
        if (!handle.IsValid() || handle.GetType() != HeapType)
            return nullptr;
        
        uint32_t index = handle.GetIndex();
        if (index < Capacity)
        {
            return &m_Descriptors[index];
        }
        return nullptr;
    }
}
