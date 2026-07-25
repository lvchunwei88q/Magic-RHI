#include "RHICommandList.h"

namespace RHI
{
    RHICommandPool::RHICommandPool(RHICmdType InType)
        : CmdType(InType) {}
    RHICommandPool::~RHICommandPool() = default;
        
    RHICommandList::RHICommandList() {}
    RHICommandList::~RHICommandList() = default;

    RHICommandQueue::RHICommandQueue(RHICmdType InType)
        : QueueType(InType) {}
    RHICommandQueue::~RHICommandQueue() = default;
}
