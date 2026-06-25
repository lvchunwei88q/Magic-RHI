#include "RHIResource.h"

namespace RHI
{       
#if RHI_ENABLE_DEBUG_INFO
        RHIResourceDebugInfo RHIResource::GetInfo() const
        {
            RHIResourceDebugInfo info{ /* NOT */ };
            return info;
        }
#endif
}