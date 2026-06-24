#pragma once

#include <RHIRootSignature.h>
#include <array>

#include <cstddef>
#include <d3d11.h>
#include <memory>
#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

// To make DX11 support root signatures, we just need to record the resources required by this root signature when setting it up,
//  and then set all the resources right before Draw.
namespace RHI
{
    /*
    * Root parameter data structure, for D3D11.
    */
    struct RootParameterD3D11
    {
        // TODO: Root parameter data
    };
    struct RootSignatureDescD3D11
    {
        std::vector<RootParameterD3D11> RootParameters;
        RootSignatureFlags Flags = RootSignatureFlags::None;
    };

    // Because DX11 doesn't allow setting root constants directly,
    // we need to use a buffer pool to keep track of these constants and then map them into the buffer.
    struct RootConstantDataD3D11
    {
        // Maximum buffer size for root constants.
        static constexpr uint32_t   MAX_BUFFER_SIZE = 256;
        // Constant buffer for root constants.
        ComPtr<ID3D11Buffer>        ConstantBuffer;
        std::array<uint8_t, MAX_BUFFER_SIZE> Cache;
        // Used size of the constant buffer.
        uint32_t                    UsedSize = 0;

        // Create constant buffer for root constants.
        HRESULT CreateBuffer(ID3D11Device* pDevice)
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = MAX_BUFFER_SIZE;            // 256 bytes
            desc.Usage = D3D11_USAGE_DYNAMIC;            // Allows CPU update frequently
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, ConstantBuffer.GetAddressOf());
            if (SUCCEEDED(hr))
                UsedSize = 0;
            return hr;
        }

        // Release constant buffer.
        void ReleaseBuffer()
        {
            ConstantBuffer.Reset();
            UsedSize = 0;
            Cache.fill(0);
        }

        // We are just borrowing this device for now, but we can't keep it forever.
        RootConstantDataD3D11() = default;
        RootConstantDataD3D11(ID3D11Device* pDevice);
        RootConstantDataD3D11(const RootConstantDataD3D11& other) {
            UsedSize = other.UsedSize;
            Cache = other.Cache;
            ConstantBuffer = other.ConstantBuffer;
        };
        ~RootConstantDataD3D11();
    };

    class RootConstantPoolD3D11
    {
    public:
        // We are just borrowing this device for now, but we can't keep it forever.
        RootConstantPoolD3D11(ID3D11Device* pDevice,size_t poolCount);
        ~RootConstantPoolD3D11();

    private:
        std::vector<RootConstantDataD3D11> m_Pools;
    };
    
    class RHIRootSignatureD3D11 : public RHIRootSignature
    {
    public:
        RHIRootSignatureD3D11();
        ~RHIRootSignatureD3D11() override;

        bool Initialize(Device* device, const RootSignatureDesc& desc) override;
        void Shutdown() override;

        bool IsValid() const override;

        // Here we set up all the assets in this root signature for it. Generally speaking, we recommend calling it before Draw.
        static void SetRootSignatureResources(RHIRootSignatureD3D11* pRootSignature, ID3D11DeviceContext* pDeviceContext);
    private:
        RootSignatureDescD3D11 m_Desc;

        // Root constant pool for root constants.
        std::unique_ptr<RootConstantPoolD3D11> m_Pool = nullptr;
    };
}
