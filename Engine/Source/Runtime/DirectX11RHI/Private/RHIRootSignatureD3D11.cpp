#include "RHID3D11.h"
#include "RHIRootSignatureD3D11.h"

namespace RHI
{
    RootConstantDataD3D11::RootConstantDataD3D11(ID3D11Device* pDevice)
    {
        CreateBuffer(pDevice);
    }
        
    RootConstantDataD3D11::~RootConstantDataD3D11()
    {
        ReleaseBuffer();
    }

    RootConstantPoolD3D11::RootConstantPoolD3D11(ID3D11Device* pDevice,size_t poolCount)
    {
        m_Pools.resize(poolCount);
        for (size_t i = 0; i < poolCount; ++i)
        {
            RootConstantDataD3D11 Data(pDevice);
            m_Pools[i] = Data;
        }
    }

    RootConstantPoolD3D11::~RootConstantPoolD3D11()
    {
        for (size_t i = 0; i < m_Pools.size(); ++i)
        {
            m_Pools[i].ReleaseBuffer();
        }
    }

    RHIRootSignatureD3D11::RHIRootSignatureD3D11()
    {
    }

    RHIRootSignatureD3D11::~RHIRootSignatureD3D11()
    {
        Shutdown();
    }

    bool RHIRootSignatureD3D11::Initialize(Device* device, const RootSignatureDesc& desc)
    {
        if (device == nullptr){
#ifdef RHI_ENABLE_RESOURCE_DEBUG_INFO
                ThrowErrorMessage("Device is nullptr");
#endif
            return false;
        }
        RHID3D11* pDevice = SafeCast<RHID3D11>(device);
        m_Desc.Flags = desc.Flags;

        // We need to create the corresponding Const Buffer based on the root constants in the root signature set by the user.
        size_t numConstants = 0;
        for (const auto& param : desc.Parameters)
        {
            // if the parameter is a root constant, we need to create a buffer for it.
            if (param.Type == RootParameterType::Constants)
                numConstants++;
        }

        m_Pool = std::make_unique<RootConstantPoolD3D11>(pDevice->GetDevice(), numConstants);
        // TODO: Root signature data
        return true;
    }

    void RHIRootSignatureD3D11::Shutdown()
    {
        m_Pool.reset();
    }

    bool RHIRootSignatureD3D11::IsValid() const
    {
        return m_Pool != nullptr;
    }

    void RHIRootSignatureD3D11::SetRootSignatureResources(RHIRootSignatureD3D11* pRootSignature, ID3D11DeviceContext* pDeviceContext)
    {
        // TODO: Set root constants
    }
}
