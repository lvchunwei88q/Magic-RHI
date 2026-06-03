#include "RHICommandList.h"

namespace RHI
{
    RHICommandAllocator::RHICommandAllocator(RHICmdType InType)
        : CmdType(InType) {}
    RHICommandAllocator::~RHICommandAllocator() = default;
        
    RHICommandList::RHICommandList(RHICommandAllocator* pCmdAllocator)
        : m_pAllocator(pCmdAllocator) {}
    RHICommandList::~RHICommandList() = default;

    RHICommandQueue::RHICommandQueue(RHICmdType InType)
        : QueueType(InType) {}
    RHICommandQueue::~RHICommandQueue() = default;
}
