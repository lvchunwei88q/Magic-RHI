#include "RHICommandListDirectX11.h"
#include "DescriptorHeapDirectX11.h"
#include "RHIRootSignatureDirectX11.h"

namespace RHI
{
    void CommandListDirectX11::SetGraphicsRootSignature(RHIRootSignature* /*pRootSignature*/)
    {
    }

    void CommandListDirectX11::SetComputeRootSignature(RHIRootSignature* /*pRootSignature*/)
    {
    }

    void CommandListDirectX11::SetDescriptorHeaps(uint32_t /*numHeaps*/, RHIDescriptorHeap* const* /*ppHeaps*/)
    {
    }

    void CommandListDirectX11::SetGraphicsRootDescriptorTable(uint32_t /*rootParameterIndex*/, RHIDescriptorHeap* /*pDescriptorHeap*/, uint32_t /*offsetInDescriptorsFromTableStart*/)
    {
    }

    void CommandListDirectX11::SetGraphicsRootConstantBufferView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListDirectX11::SetGraphicsRootShaderResourceView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListDirectX11::SetGraphicsRootUnorderedAccessView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListDirectX11::SetGraphicsRoot32BitConstant(uint32_t /*rootParameterIndex*/, uint32_t /*value*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }

    void CommandListDirectX11::SetGraphicsRoot32BitConstants(uint32_t /*rootParameterIndex*/, uint32_t /*num32BitValues*/, const void* /*pSrcData*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }

    void CommandListDirectX11::SetComputeRootDescriptorTable(uint32_t /*rootParameterIndex*/, RHIDescriptorHeap* /*pDescriptorHeap*/, uint32_t /*offsetInDescriptorsFromTableStart*/)
    {
    }

    void CommandListDirectX11::SetComputeRootConstantBufferView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListDirectX11::SetComputeRootShaderResourceView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListDirectX11::SetComputeRootUnorderedAccessView(uint32_t /*rootParameterIndex*/, uint64_t /*gpuVirtualAddress*/)
    {
    }

    void CommandListDirectX11::SetComputeRoot32BitConstant(uint32_t /*rootParameterIndex*/, uint32_t /*value*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }

    void CommandListDirectX11::SetComputeRoot32BitConstants(uint32_t /*rootParameterIndex*/, uint32_t /*num32BitValues*/, const void* /*pSrcData*/, uint32_t /*destOffsetIn32BitValues*/)
    {
    }
}
