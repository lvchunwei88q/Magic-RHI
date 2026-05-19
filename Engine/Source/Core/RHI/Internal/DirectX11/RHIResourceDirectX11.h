#pragma once
#include <d3d11.h>
#include "DirectXHelper.h"
#include "RHIResource.h"

#include <wrl.h> // ComPtr
using Microsoft::WRL::ComPtr;

namespace RHI
{
    class SamplerStateDirectX11 : public RHISamplerState
    {
    public:
        SamplerStateDirectX11(ID3D11SamplerState* pSamplerState) : m_pSamplerState(pSamplerState),
         RHISamplerState({RHIDescriptorHeapType::Sampler, 0}) {}
        ~SamplerStateDirectX11() override = default;

        ID3D11SamplerState* GetSamplerState() const { return m_pSamplerState.Get(); }

    private:
        ComPtr<ID3D11SamplerState> m_pSamplerState;
    };

    class BufferDirectX11 : public RHIBuffer
    {
    public:
        BufferDirectX11(ID3D11Buffer* pBuffer, const BufferDesc& InDesc)
            : RHIBuffer(InDesc, RRT_Buffer)
            , m_pBuffer(pBuffer)
            , m_DeviceContext(nullptr)
        {
        }

        ~BufferDirectX11() override = default;

        void SetDeviceContext(ID3D11DeviceContext* InDeviceContext)
        {
            m_DeviceContext = InDeviceContext;
        }

        ID3D11Buffer* GetBuffer() const { return m_pBuffer.Get(); }

        void* Map() override
        {
            if (m_DeviceContext && GetHeapType() != BufferHeapType::Default)
            {
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                HRESULT hr = m_DeviceContext->Map(m_pBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                if (SUCCEEDED(hr))
                {
                    return mappedResource.pData;
                }
            }
#if RHI_ENABLE_RESOURCE_INFO
            else if(GetHeapType() == BufferHeapType::Default) 
                ThrowIfFailed("Failed to map default buffer");
#endif
            return nullptr;
        }

        void Unmap() override
        {
            if (m_DeviceContext && GetHeapType() != BufferHeapType::Default)
            {
                m_DeviceContext->Unmap(m_pBuffer.Get(), 0);
            }
#if RHI_ENABLE_RESOURCE_INFO
            else if(GetHeapType() == BufferHeapType::Default) 
                ThrowIfFailed("Failed to unmap default buffer");
#endif
        }

    private:
        ComPtr<ID3D11Buffer> m_pBuffer;
        ID3D11DeviceContext* m_DeviceContext;
    };

    class CommandListDirectX11 : public RHICommandList
    {
    public:
        CommandListDirectX11(RHICmdListType InType)
            : RHICommandList(InType) {}
        ~CommandListDirectX11() override = default;

        /* DX 11 不支持命令列表记录 */ 
        void BeginRecording() override {}
        void EndRecording() override {}
    };

    class CommandQueueDirectX11 : public RHICommandQueue
    {
    public:
        CommandQueueDirectX11(RHICmdListType InType, ID3D11DeviceContext* pContext)
            : RHICommandQueue(InType)
            , m_pDeviceContext(pContext) {}
        ~CommandQueueDirectX11() override = default;

        void ExecuteCommandLists(const std::vector<std::shared_ptr<RHICommandList>>& cmdLists) override {}
        void WaitForIdle() override {}

        // 同步操作 但是 DX 11 不支持
        uint64_t Signal() override { return 0; }
        bool GetTimestampFrequency(uint64_t* frequency) override { return false; }
        bool SetEventOnCompletion(uint64_t fenceValue, void* hEvent) override { return false; }
        uint64_t GetCompletedValue() const override { return 0; }
    private:
        ID3D11DeviceContext* m_pDeviceContext;
    };
}
