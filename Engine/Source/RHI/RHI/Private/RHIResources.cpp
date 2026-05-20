#include "RHIResource.h"

namespace RHI
{       
#if RHI_ENABLE_RESOURCE_INFO
        void RHIResource::GetInfo(RHIResourceInfo& OutInfo) const{
            OutInfo = RHIResourceInfo{ /* NOT */ };
        }
#endif
}