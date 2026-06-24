#include "RHICommandListD3D11.h"
#include "DescriptorHeapD3D11.h"
#include "RHIRootSignatureD3D11.h"

namespace RHI
{
    void CommandListD3D11::SetGraphicsRootSignature(RHIRootSignature* pRootSignature)
    {
        m_tempBindingAssignment.m_pRootSignature = SafeCast<RHIRootSignatureD3D11>(pRootSignature);
        // Set the root signature draw callback.
        CommandDrawCallbackD3D11::Get().SetCallback(RHIRootSignatureD3D11::SetRootSignatureResources);
    }

    void CommandListD3D11::SetComputeRootSignature(RHIRootSignature* pRootSignature)
    {
        m_tempBindingAssignment.m_pRootSignature = SafeCast<RHIRootSignatureD3D11>(pRootSignature);
        // Set the root signature draw callback.
        CommandDrawCallbackD3D11::Get().SetCallback(RHIRootSignatureD3D11::SetRootSignatureResources);
    }

    void CommandListD3D11::SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps)
    {
        // Resize the descriptor heap vector.
        m_tempBindingAssignment.m_ppHeaps.resize(numHeaps);
        for (uint32_t i = 0; i < numHeaps; ++i)
        {
            m_tempBindingAssignment.m_ppHeaps[i] = SafeCast<DescriptorHeapD3D11>(ppHeaps[i]);
        }
    }

    void CommandListD3D11::SetGraphicsRootDescriptorTable(uint32_t /*rootParameterIndex*/, RHIDescriptorHeap* /*pDescriptorHeap*/, uint32_t /*offsetInDescriptorsFromTableStart*/)
    {
    }

    void CommandListD3D11::SetGraphicsRootConstantBufferView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListD3D11::SetGraphicsRootShaderResourceView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListD3D11::SetGraphicsRootUnorderedAccessView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListD3D11::SetGraphicsRoot32BitConstant(uint32_t /*rootParameterIndex*/, uint32_t /*value*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }

    void CommandListD3D11::SetGraphicsRoot32BitConstants(uint32_t /*rootParameterIndex*/, uint32_t /*num32BitValues*/, const void* /*pSrcData*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }

    void CommandListD3D11::SetComputeRootDescriptorTable(uint32_t /*rootParameterIndex*/, RHIDescriptorHeap* /*pDescriptorHeap*/, uint32_t /*offsetInDescriptorsFromTableStart*/)
    {
    }

    void CommandListD3D11::SetComputeRootConstantBufferView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListD3D11::SetComputeRootShaderResourceView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListD3D11::SetComputeRootUnorderedAccessView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListD3D11::SetComputeRoot32BitConstant(uint32_t /*rootParameterIndex*/, uint32_t /*value*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }

    void CommandListD3D11::SetComputeRoot32BitConstants(uint32_t /*rootParameterIndex*/, uint32_t /*num32BitValues*/, const void* /*pSrcData*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }
}
