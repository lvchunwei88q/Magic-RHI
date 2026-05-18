#pragma once

#include "Common/RHI_API.h"
#include "Common/RHIResourceType.h"
#include <string>
#include <atomic>

#ifndef RHI_ENABLE_RESOURCE_INFO
    #ifdef _DEBUG
        #define RHI_ENABLE_RESOURCE_INFO 1   // Debug: Run
    #else
        #define RHI_ENABLE_RESOURCE_INFO 0   // Release: Close
    #endif
#endif

namespace RHI
{  
    struct GPUVRamAllocation
    {
        GPUVRamAllocation() = default;
        GPUVRamAllocation(uint64_t InAllocationStart, uint64_t InAllocationSize)
            : AllocationStart(InAllocationStart)
            , AllocationSize(InAllocationSize)
        {
        }

        bool IsValid() const { return AllocationSize > 0; }
        
        // in bytes
        uint64_t AllocationStart{};
        // in bytes
        uint64_t AllocationSize{};
    };

    struct RHIResourceInfo
    {
        std::string Name;
        RHIResourceType Type{ RRT_None };
        GPUVRamAllocation VRamAllocation;
        bool IsTransient{ false };
        bool bValid{ true };
        bool bResident{ true };
    };

    /** The base type of RHI resources. */
    class RHI_API RHIResource
    {
    public:
        RHIResource(RHIResourceType InType) : Type(InType) {}
        virtual ~RHIResource() = default;
        
        // Copying is prohibited
        RHIResource(const RHIResource&) = delete;
        RHIResource& operator=(const RHIResource&) = delete;
        
        // Reference counting
        uint32_t AddRef()
        {
            return ++RefCount;
        }
        
        uint32_t Release()
        {
            uint32_t NewCount = --RefCount;
            if (NewCount == 0)
            {
                delete this;
            }
            return NewCount;
        }
        
        uint32_t GetRefCount() const { return RefCount; }
        
        // type
        RHIResourceType GetType() const { return Type; }
        
#if RHI_ENABLE_RESOURCE_INFO
        virtual void GetInfo(RHIResourceInfo& OutInfo) const;
#endif

    protected:
        std::atomic<uint32_t> RefCount{ 1 }; 
        RHIResourceType Type;
    };
} // namespace RHI