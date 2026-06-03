#include "RHICommandListDirectX11.h"

namespace RHI
{
    void CommandListDirectX11::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->Draw(vertexCount, startVertexLocation);
    }

    void CommandListDirectX11::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void CommandListDirectX11::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX11::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX11::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        CommandAllocatorDirectX11* pAllocator = SafeCast<CommandAllocatorDirectX11>(m_pAllocator);
        pAllocator->GetDeviceContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }
}
