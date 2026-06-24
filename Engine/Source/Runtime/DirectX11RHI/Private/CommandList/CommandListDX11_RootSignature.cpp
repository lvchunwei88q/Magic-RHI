#include "RHICommandListD3D11.h"
#include "DescriptorHeapD3D11.h"
#include "RHIRootSignatureD3D11.h"

#define SET_ROOT_CONSTANT_BUFFER(rootParameterIndex, num32BitValues, srcData, destOffsetIn32BitValues)                              \
    CommandAllocatorD3D11* pAllocator = GetAllocator();                                                                             \
    ID3D11DeviceContext* pDeviceContext = pAllocator->GetDeviceContext();                                                           \
    RootConstantDataD3D11* pRootConstantData = m_tempBindingAssignment.m_pRootSignature->GetRootConstantData(rootParameterIndex);   \
    /* Update the constant buffer. */                                                                                               \
    UpdateRootConstantBuffer(pRootConstantData, pDeviceContext, num32BitValues, srcData, destOffsetIn32BitValues);

namespace RHI
{
    namespace{
        auto UpdateRootConstantBuffer = [](RootConstantDataD3D11* pRootConstantData, ID3D11DeviceContext* pDeviceContext, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues) -> void {
            // Check if the root constant data is valid.
            if (pRootConstantData == nullptr || pDeviceContext == nullptr)
                return;

            // Calculate the data size in bytes.
            size_t dataSizeInBytes = sizeof(uint32_t) * num32BitValues;

            // Check if the constant buffer is consistent with the cache.
            if (!pRootConstantData->IsConsistentWithCache(pSrcData, dataSizeInBytes, destOffsetIn32BitValues))
                // Update the constant buffer.
                pRootConstantData->UpdateBuffer(pDeviceContext, pSrcData, dataSizeInBytes, destOffsetIn32BitValues);
        };
    }

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

    void CommandListD3D11::SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues)
    {
        SET_ROOT_CONSTANT_BUFFER(rootParameterIndex, 1, &value, destOffsetIn32BitValues);
    }

    void CommandListD3D11::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues)
    {
        SET_ROOT_CONSTANT_BUFFER(rootParameterIndex, num32BitValues, pSrcData, destOffsetIn32BitValues);
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

    void CommandListD3D11::SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t value, uint32_t destOffsetIn32BitValues)
    {
        SET_ROOT_CONSTANT_BUFFER(rootParameterIndex, 1, &value, destOffsetIn32BitValues);
    }

    void CommandListD3D11::SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValues, const void* pSrcData, uint32_t destOffsetIn32BitValues)
    {
        SET_ROOT_CONSTANT_BUFFER(rootParameterIndex, num32BitValues, pSrcData, destOffsetIn32BitValues);
    }
}
