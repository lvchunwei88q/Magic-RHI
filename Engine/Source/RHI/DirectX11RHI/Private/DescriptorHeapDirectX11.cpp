#include "DescriptorHeapDirectX11.h"
#include "RHIDirectX11.h"
#include "DirectXHelper.h"

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
        
        m_FreeList.push_back(index);
        CurrentIndex = Capacity - static_cast<uint32_t>(m_FreeList.size());
    }
}
