#pragma once

#include <RHIRootSignature.h>

namespace RHI
{
    struct RootParameterD3D11
    {
        // TODO: Root parameter data
    };
    struct RootSignatureDescD3D11
    {
        std::vector<RootParameterD3D11> RootParameters;
        RootSignatureFlags Flags = RootSignatureFlags::None;
    };
    
    class RHIRootSignatureD3D11 : public RHIRootSignature
    {
    public:
        RHIRootSignatureD3D11();
        ~RHIRootSignatureD3D11() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

    private:
        RootSignatureDescD3D11 m_Desc;
    };
}
