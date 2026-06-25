#include "RHICommandListD3D12.h"
#include "DescriptorHeapD3D12.h"
#include "RHIRootSignatureD3D12.h"

namespace RHI
{
    void CommandListD3D12::SetGraphicsRootSignature(RHIRootSignature* pRootSignature)
    {
        if (!pRootSignature)
        {
            m_pCommandList->SetGraphicsRootSignature(nullptr);
            return;
        }

        auto* pDX12RootSignature = SafeCast<RHIRootSignatureD3D12>(pRootSignature);
        m_pCommandList->SetGraphicsRootSignature(pDX12RootSignature->GetRootSignature());
    }

    void CommandListD3D12::SetComputeRootSignature(RHIRootSignature* pRootSignature)
    {
        if (!pRootSignature)
        {
            m_pCommandList->SetComputeRootSignature(nullptr);
            return;
        }

        auto* pDX12RootSignature = SafeCast<RHIRootSignatureD3D12>(pRootSignature);
        m_pCommandList->SetComputeRootSignature(pDX12RootSignature->GetRootSignature());
    }

    void CommandListD3D12::SetDescriptorHeaps(uint32_t numHeaps, RHIDescriptorHeap* const* ppHeaps)
    {
        if (numHeaps == 0 || !ppHeaps)
        {
            return;
        }

        std::vector<ID3D12DescriptorHeap*> d3dHeaps(numHeaps);
        for (uint32_t i = 0; i < numHeaps; ++i)
        {
            d3dHeaps[i] = SafeCast<DescriptorHeapD3D12>(ppHeaps[i])->GetHeap();
        }

        m_pCommandList->SetDescriptorHeaps(static_cast<UINT>(numHeaps), d3dHeaps.data());
    }

    void CommandListD3D12::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart)
    {
        if (!pDescriptorHeap)
        {
            return;
        }

        auto* pDX12Heap = SafeCast<DescriptorHeapD3D12>(pDescriptorHeap);
        D3D12_GPU_DESCRIPTOR_HANDLE handle = pDX12Heap->GetGPUHandle(offsetInDescriptorsFromTableStart);
        m_pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, handle);
    }

    void CommandListD3D12::SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
        m_pCommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, gpuVirtualAddress);
    }

    void CommandListD3D12::SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
        m_pCommandList->SetGraphicsRootShaderResourceView(rootParameterIndex, gpuVirtualAddress);
    }

    void CommandListD3D12::SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
        m_pCommandList->SetGraphicsRootUnorderedAccessView(rootParameterIndex, gpuVirtualAddress);
    }

    void CommandListD3D12::SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues)
    {
        m_pCommandList->SetGraphicsRoot32BitConstant(rootParameterIndex, value, destOffsetIn32BitValues);
    }

    void CommandListD3D12::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues)
    {
        m_pCommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValues, pSrcData, destOffsetIn32BitValues);
    }

    void CommandListD3D12::SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RHIDescriptorHeap* pDescriptorHeap, uint32_t offsetInDescriptorsFromTableStart)
    {
        if (!pDescriptorHeap)
        {
            return;
        }

        auto* pDX12Heap = SafeCast<DescriptorHeapD3D12>(pDescriptorHeap);
        D3D12_GPU_DESCRIPTOR_HANDLE handle = pDX12Heap->GetGPUHandle(offsetInDescriptorsFromTableStart);
        m_pCommandList->SetComputeRootDescriptorTable(rootParameterIndex, handle);
    }

    void CommandListD3D12::SetComputeRootConstantBufferView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
        m_pCommandList->SetComputeRootConstantBufferView(rootParameterIndex, gpuVirtualAddress);
    }

    void CommandListD3D12::SetComputeRootShaderResourceView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
        m_pCommandList->SetComputeRootShaderResourceView(rootParameterIndex, gpuVirtualAddress);
    }

    void CommandListD3D12::SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, uint64_t gpuVirtualAddress)
    {
        m_pCommandList->SetComputeRootUnorderedAccessView(rootParameterIndex, gpuVirtualAddress);
    }

    void CommandListD3D12::SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues)
    {
        m_pCommandList->SetComputeRoot32BitConstant(rootParameterIndex, value, destOffsetIn32BitValues);
    }

    void CommandListD3D12::SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues)
    {
        m_pCommandList->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValues, pSrcData, destOffsetIn32BitValues);
    }
}
