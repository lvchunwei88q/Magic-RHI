#include <Common/RHIException.h>
#include "DescriptorHeapDirectX11.h"
#include "RHIDirectX11.h"

#define IMPLEMENT_SET_DESCRIPTOR(Type, Member, ErrorFunc)               \
        if (!handle.IsValid() || handle.GetType() != HeapType)          \
            return;                                                     \
        uint32_t index = handle.GetIndex();                             \
        if (index < Capacity)                                           \
        {                                                               \
            m_Descriptors[index].Type = Member;                         \
        }                                                               \
        else                                                            \
        {                                                               \
            auto error = ErrorFunc;                                     \
            error();                                                    \
        }

namespace RHI
{
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
        
        m_Descriptors[index].pGeneric = nullptr;
        m_FreeList.push_back(index);
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, BufferDirectX11* pBuffer)
    {
        IMPLEMENT_SET_DESCRIPTOR(pBuffer, pBuffer,[](){
            ThrowErrorMessage("Error SetDescriptor Buffer Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, ShaderResourceViewDirectX11* pSRV)
    {
        IMPLEMENT_SET_DESCRIPTOR(pSRV, pSRV,[](){
            ThrowErrorMessage("Error SetDescriptor ShaderResourceView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, UnorderedAccessViewDirectX11* pUAV)
    {
        IMPLEMENT_SET_DESCRIPTOR(pUAV, pUAV,[](){
            ThrowErrorMessage("Error SetDescriptor UnorderedAccessView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, RenderTargetViewDirectX11* pRTV)
    {
        IMPLEMENT_SET_DESCRIPTOR(pRTV, pRTV,[](){
            ThrowErrorMessage("Error SetDescriptor RenderTargetView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, DepthStencilViewDirectX11* pDSV)
    {
        IMPLEMENT_SET_DESCRIPTOR(pDSV, pDSV,[](){
            ThrowErrorMessage("Error SetDescriptor DepthStencilView Unknown index range");
        });
    }

    void DescriptorHeapDirectX11::SetDescriptor(RHIDescriptorHandle handle, SamplerStateDirectX11* pSampler)
    {
        IMPLEMENT_SET_DESCRIPTOR(pSampler, pSampler,[](){
            ThrowErrorMessage("Error SetDescriptor SamplerState Unknown index range");
        });
    }

    DescriptorData DescriptorHeapDirectX11::GetDescriptor(RHIDescriptorHandle handle) const
    {
        DescriptorData data = {};
        data.pGeneric = nullptr;
        
        if (!handle.IsValid() || handle.GetType() != HeapType)
            return data;
        
        uint32_t index = handle.GetIndex();
        if (index < Capacity)
        {
            return m_Descriptors[index];
        }
        return data;
    }
}
