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
        RootParameterType Type;                                 // Root parameter type
        ShaderVisibility Visibility = ShaderVisibility::All;    // Visibility

        union
        {
            // ===== Root Descriptor =====
            struct
            {
                uint32_t ShaderRegister;// Shader register start index
                // We can't support namespaces in DX11 here.
                //uint32_t RegisterSpace; // Register space (space0 , space1 , space2 , ...)
            } Descriptor;

            // ===== Root Constants =====
            struct
            {
                uint32_t ShaderRegister;// Shader register start index
                // We can't support namespaces in DX11 here.
                //uint32_t RegisterSpace; // Register space (space0 , space1 , space2 , ...)
            } Constants;

            // TODO: Root parameter data
        };
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
        // Location of the root constant in the root signature.
        size_t                      Location = 0;

        // Create constant buffer for root constants.
        HRESULT CreateBuffer(ID3D11Device* pDevice);
        // Update constant buffer with new data.
        HRESULT UpdateBuffer(ID3D11DeviceContext* pContext, const void* pData, uint32_t sizeInBytes, uint32_t destOffsetIn32BitValues);
        // Check data if it Consistent with the cache.
        bool IsConsistentWithCache(const void* pData, uint32_t sizeInBytes, uint32_t destOffsetIn32BitValues) const;
        
        // Release constant buffer.
        void ReleaseBuffer()
        {
            ConstantBuffer.Reset();
            UsedSize = 0; Location = 0;
            Cache.fill(0);
        }

        // We are just borrowing this device for now, but we can't keep it forever.
        RootConstantDataD3D11() = default;
        RootConstantDataD3D11(ID3D11Device* pDevice,size_t location);
        RootConstantDataD3D11(const RootConstantDataD3D11& other) {
            UsedSize = other.UsedSize; Location = other.Location;
            Cache = other.Cache;
            ConstantBuffer = other.ConstantBuffer;
        };
        ~RootConstantDataD3D11();
    };

    // Indicates the position of the root constant in the root signature
    struct ConstantSignatureLocation
    {
        std::vector<size_t> Location;
    };

    class RootConstantPoolD3D11
    {
    public:
        // We are just borrowing this device for now, but we can't keep it forever.
        RootConstantPoolD3D11(ID3D11Device* pDevice,ConstantSignatureLocation locations);
        ~RootConstantPoolD3D11();

        size_t GetSize() const { return m_Pools.size(); }
        const std::vector<RootConstantDataD3D11>& GetPools() const { return m_Pools; }
        RootConstantDataD3D11* GetRootConstantData(size_t index) { return &m_Pools[index]; }
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

        // Get root constant data for root constants.
        // @param rootParameterIndex Index of root parameter.
        RootConstantDataD3D11* GetRootConstantData(uint32_t rootParameterIndex) const;

        // Here we set up all the assets in this root signature for it. Generally speaking, we recommend calling it before Draw.
        static void SetRootSignatureResources(RHIRootSignatureD3D11* pRootSignature, ID3D11DeviceContext* pDeviceContext);
    private:
        RootSignatureDescD3D11 m_Desc;

        // Root constant pool for root constants.
        std::unique_ptr<RootConstantPoolD3D11> m_Pool = nullptr;
    };
}
