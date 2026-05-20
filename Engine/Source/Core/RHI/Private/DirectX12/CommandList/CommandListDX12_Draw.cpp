#include "DirectX12/RHIResourceDirectX12.h"

namespace RHI
{
    void CommandListDirectX12::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        m_pCommandList->DrawInstanced(vertexCount, 1, startVertexLocation, 0);
    }

    void CommandListDirectX12::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        m_pCommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
    }

    void CommandListDirectX12::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        m_pCommandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX12::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        m_pCommandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX12::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        m_pCommandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }
}
