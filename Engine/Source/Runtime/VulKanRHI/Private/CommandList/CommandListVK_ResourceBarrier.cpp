#include <CoreMinimal.h>
#include "RHICommandListVulKan.h"

namespace RHI
{
    namespace
    {
    }

    void CommandListVulKan::ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers)
    {
        if (numBarriers == 0 || pBarriers == nullptr)
            return;
        
        // TODO: 实现资源屏障
    }
}
