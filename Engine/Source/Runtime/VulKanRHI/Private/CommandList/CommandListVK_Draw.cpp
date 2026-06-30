#include "RHICommandListVulKan.h"

namespace RHI
{
    void CommandListVulKan::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        vkCmdDraw(m_CommandBuffer, vertexCount, 1, startVertexLocation, 0);
    }

    void CommandListVulKan::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        vkCmdDrawIndexed(m_CommandBuffer, indexCount, 1, startIndexLocation, baseVertexLocation, 0);
    }

    void CommandListVulKan::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        //vkCmdDrawInstanced(m_CommandBuffer, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void CommandListVulKan::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        //vkCmdDrawIndexedInstanced(m_CommandBuffer, indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void CommandListVulKan::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        vkCmdDispatch(m_CommandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }
}
