#include <RHIRootSignatureDirectX11.h>
// DX11 不需要真正的根签名，返回一个空操作的根签名对象

namespace RHI
{
    RHIRootSignatureDirectX11::RHIRootSignatureDirectX11()
    {
    }

    RHIRootSignatureDirectX11::~RHIRootSignatureDirectX11()
    {
        Shutdown();
    }

    bool RHIRootSignatureDirectX11::Initialize(Device* device, const RootSignatureDesc& desc)
    {
        
        return true;
    }

    void RHIRootSignatureDirectX11::Shutdown()
    {
    }

    bool RHIRootSignatureDirectX11::IsValid() const
    {
        return true;
    }
}
