#include "RHICommandListD3D11.h"

namespace RHI
{
    void CommandListD3D11::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
        CommandDrawCallbackD3D11::Get().Execute(m_tempBindingAssignment.m_pRootSignature, pAllocator->GetDeviceContext());
        pAllocator->GetDeviceContext()->Draw(vertexCount, startVertexLocation);
    }

    void CommandListD3D11::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
        CommandDrawCallbackD3D11::Get().Execute(m_tempBindingAssignment.m_pRootSignature, pAllocator->GetDeviceContext());
        pAllocator->GetDeviceContext()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void CommandListD3D11::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
        CommandDrawCallbackD3D11::Get().Execute(m_tempBindingAssignment.m_pRootSignature, pAllocator->GetDeviceContext());
        pAllocator->GetDeviceContext()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListD3D11::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
        CommandDrawCallbackD3D11::Get().Execute(m_tempBindingAssignment.m_pRootSignature, pAllocator->GetDeviceContext());
        pAllocator->GetDeviceContext()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListD3D11::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        CommandAllocatorD3D11* pAllocator = SafeCast<CommandAllocatorD3D11>(m_pAllocator);
        CommandDrawCallbackD3D11::Get().Execute(m_tempBindingAssignment.m_pRootSignature, pAllocator->GetDeviceContext());
        pAllocator->GetDeviceContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }
}
