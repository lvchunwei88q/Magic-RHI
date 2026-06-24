#include "RHIRootSignatureD3D11.h"
// DX11 不需要真正的根签名，返回一个空操作的根签名对象

namespace RHI
{
    RHIRootSignatureD3D11::RHIRootSignatureD3D11()
    {
    }

    RHIRootSignatureD3D11::~RHIRootSignatureD3D11()
    {
        Shutdown();
    }

    bool RHIRootSignatureD3D11::Initialize(Device* device, const RootSignatureDesc& desc)
    {
        m_Desc.Flags = desc.Flags;
        // TODO: Root signature data
        return true;
    }

    void RHIRootSignatureD3D11::Shutdown()
    {
    }

    bool RHIRootSignatureD3D11::IsValid() const
    {
        return true;
    }
}
