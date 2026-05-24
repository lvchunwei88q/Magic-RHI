#pragma once

#include "Common/DIRECTX11RHI_API.h"
#include <RHIRootSignature.h>

namespace RHI
{
    class DIRECTX11RHI_API RHIRootSignatureDirectX11 : public RHIRootSignature
    {
    public:
        RHIRootSignatureDirectX11();
        ~RHIRootSignatureDirectX11() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;
    };
}
