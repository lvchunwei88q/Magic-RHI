#include <Common/Check.h>
#include "DescriptorHeapDirectX11.h"
#include "RHIDirectX11.h"

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
    RHIResource* DescriptorHeapDirectX11::GetDescriptorHeapView(RHIDescriptorHandle handle) const
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

    RHIDescriptorHandle DescriptorHeapDirectX11::Allocate()
    {
        if (IsFull())
        {
            ThrowErrorMessage("Descriptor heap is full");
            return RHIDescriptorHandle();
        }

        uint32_t index = m_FreeList.back();
        m_FreeList.pop_back();
        
        // 更新已分配数量
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
        
        return RHIDescriptorHandle(HeapType, index);
    }

    void DescriptorHeapDirectX11::Free(RHIDescriptorHandle handle)
    {
        if (!handle.IsValid() || handle.GetType() != HeapType)
        {
            return;
        }
        
        uint32_t index = handle.GetIndex();
        
        // 验证索引范围
        if (index >= Capacity)
        {
            ThrowErrorMessage("Error Unknown index range");
            return;
        }
        
        // 防止重复释放
#if _DEBUG
        auto it = std::find(m_FreeList.begin(), m_FreeList.end(), index);
        if (it != m_FreeList.end())
        {
            // 已经空闲，重复释放
            ThrowErrorMessage("Double free detected");
            return;
        }
#endif
        
        m_Descriptors[index].Release();
        m_FreeList.push_back(index);
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, ConstantBufferViewDirectX11* pCBV)
    {
        IMPLEMENT_SET_DESCRIPTOR(ConstantBufferViewDirectX11, pCBV, RHIResourceType::RRT_ConstantBufferView,[](){
            ThrowErrorMessage("Error SetDescriptor ConstantBufferView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, ShaderResourceViewDirectX11* pSRV)
    {
        IMPLEMENT_SET_DESCRIPTOR(ShaderResourceViewDirectX11, pSRV, RHIResourceType::RRT_ShaderResourceView,[](){
            ThrowErrorMessage("Error SetDescriptor ShaderResourceView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewDirectX11* pUAV)
    {
        IMPLEMENT_SET_DESCRIPTOR(UnorderedAccessViewDirectX11, pUAV, RHIResourceType::RRT_UnorderedAccessView,[](){
            ThrowErrorMessage("Error SetDescriptor UnorderedAccessView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, RenderTargetViewDirectX11* pRTV)
    {
        IMPLEMENT_SET_DESCRIPTOR(RenderTargetViewDirectX11, pRTV, RHIResourceType::RRT_RenderTargetView,[](){
            ThrowErrorMessage("Error SetDescriptor RenderTargetView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, DepthStencilViewDirectX11* pDSV)
    {
        IMPLEMENT_SET_DESCRIPTOR(DepthStencilViewDirectX11, pDSV, RHIResourceType::RRT_DepthStencilView,[](){
            ThrowErrorMessage("Error SetDescriptor DepthStencilView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, SamplerStateDirectX11* pSampler)
    {
        IMPLEMENT_SET_DESCRIPTOR(SamplerStateDirectX11, pSampler, RHIResourceType::RRT_SamplerState,[](){
            ThrowErrorMessage("Error SetDescriptor SamplerState Unknown index range");
        });
    }

    const DescriptorData* DescriptorHeapDirectX11::GetDescriptor(RHIDescriptorHandle handle) const
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
