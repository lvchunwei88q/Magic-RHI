#pragma once

#include <RHIRootSignature.h>

namespace RHI
{
    struct RootParameterDirectX11
    {
        // TODO: Root parameter data
    };
    struct RootSignatureDescDirectX11
    {
        std::vector<RootParameterDirectX11> RootParameters;
        RootSignatureFlags Flags = RootSignatureFlags::None;
    };
    
    class RHIRootSignatureDirectX11 : public RHIRootSignature
    {
    public:
        RHIRootSignatureDirectX11();
        ~RHIRootSignatureDirectX11() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

    private:
        RootSignatureDescDirectX11 m_Desc;
    };
}
