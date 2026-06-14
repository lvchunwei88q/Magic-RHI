#pragma once

#include <RHIRootSignature.h>

namespace RHI
{
    class RHIRootSignatureDirectX11 : public RHIRootSignature
    {
    public:
        RHIRootSignatureDirectX11();
        ~RHIRootSignatureDirectX11() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

    private:
        // TODO: Root signature data
        RootSignatureDesc m_Desc;
    };
}
