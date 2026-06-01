#include "RHICommandList.h"

namespace RHI
{
    RHICommandList::RHICommandList(RHICmdListType InType)
        : CmdListType(InType) {}
    RHICommandList::~RHICommandList() = default;

    RHICommandQueue::RHICommandQueue(RHICmdListType InType)
        : QueueType(InType) {}
    RHICommandQueue::~RHICommandQueue() = default;
}
