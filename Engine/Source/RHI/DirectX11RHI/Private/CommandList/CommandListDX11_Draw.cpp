#include "RHIResourceDirectX11.h"

namespace RHI
{
    void CommandListDirectX11::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        m_pDeviceContext->Draw(vertexCount, startVertexLocation);
    }

    void CommandListDirectX11::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        m_pDeviceContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    void CommandListDirectX11::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        m_pDeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX11::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        m_pDeviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListDirectX11::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        m_pDeviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }
}
