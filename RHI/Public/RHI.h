#pragma once

#include <Common/RHI_API.h>

#include <memory>
#include <string>

namespace RHI
{
    enum class RHIType
    {
        DirectX11,
        DirectX12
    };

    class RHI_API Device
    {
    public:
        virtual ~Device() = default;

        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;

        virtual bool IsValid() const = 0;
        virtual RHIType GetType() const = 0;
        virtual const std::wstring& GetAdapterName() const = 0;

        static std::unique_ptr<Device> Create(RHIType type);
    };
}
