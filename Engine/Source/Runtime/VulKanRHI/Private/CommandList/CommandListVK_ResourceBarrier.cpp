#include <CoreMinimal.h>
#include "RHICommandListVulKan.h"

namespace RHI
{
    namespace
    {
        // Bitwise Analysis: Converting RHI State to Vulkan's Image Layout
        VkImageLayout ConvertToVkImageLayout(RHIResourceState stateAfter)
        {
            if ((stateAfter & RHIResourceState::Present) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            if ((stateAfter & RHIResourceState::RenderTarget) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                
            if ((stateAfter & RHIResourceState::DepthWrite) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                
            if ((stateAfter & RHIResourceState::DepthRead) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                
            if ((stateAfter & (RHIResourceState::CopyDest | RHIResourceState::ResolveDest)) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                
            if ((stateAfter & (RHIResourceState::CopySource | RHIResourceState::ResolveSource)) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                
            if ((stateAfter & RHIResourceState::AllShaderResource) != RHIResourceState::Common)
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            return VK_IMAGE_LAYOUT_GENERAL; // backstop
        }

        // Bitwise Analysis: Converting RHI States to Vulkan Access Flags
        VkAccessFlags ConvertToVkAccessFlags(RHIResourceState state)
        {
            VkAccessFlags flags = 0;

            if ((state & RHIResourceState::VertexBuffer) != RHIResourceState::Common)       flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            if ((state & RHIResourceState::IndexBuffer) != RHIResourceState::Common)        flags |= VK_ACCESS_INDEX_READ_BIT;
            if ((state & RHIResourceState::RenderTarget) != RHIResourceState::Common)       flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            if ((state & RHIResourceState::UnorderedAccess) != RHIResourceState::Common)    flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            
            if ((state & RHIResourceState::DepthWrite) != RHIResourceState::Common)         flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            else if ((state & RHIResourceState::DepthRead) != RHIResourceState::Common)     flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

            if ((state & RHIResourceState::AllShaderResource) != RHIResourceState::Common)   flags |= VK_ACCESS_SHADER_READ_BIT;

            if ((state & RHIResourceState::IndirectArgument) != RHIResourceState::Common)   flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            if ((state & (RHIResourceState::CopyDest | RHIResourceState::ResolveDest)) != RHIResourceState::Common)     flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            if ((state & (RHIResourceState::CopySource | RHIResourceState::ResolveSource)) != RHIResourceState::Common) flags |= VK_ACCESS_TRANSFER_READ_BIT;

            // The Present state usually means the memory is being read (read by the rendering engine)
            if ((state & RHIResourceState::Present) != RHIResourceState::Common)            flags |= VK_ACCESS_MEMORY_READ_BIT;

            return (flags == 0) ? (VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT) : flags;
        }

        // Bitwise Analysis: Converting RHI States to Vulkan Pipeline Stages
         VkPipelineStageFlags ConvertToVkPipelineStageFlags(RHIResourceState state)
        {
            VkPipelineStageFlags flags = 0;

            if ((state & (RHIResourceState::VertexBuffer | RHIResourceState::IndexBuffer)) != RHIResourceState::Common)        
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            
            if ((state & RHIResourceState::RenderTarget) != RHIResourceState::Common)       
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            
            if ((state & (RHIResourceState::DepthWrite | RHIResourceState::DepthRead)) != RHIResourceState::Common)          
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            
            if ((state & RHIResourceState::PixelShaderResource) != RHIResourceState::Common)    
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            if ((state & RHIResourceState::NonPixelShaderResource) != RHIResourceState::Common) 
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            if ((state & RHIResourceState::UnorderedAccess) != RHIResourceState::Common)        
                flags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            if ((state & (RHIResourceState::CopyDest | RHIResourceState::CopySource | RHIResourceState::ResolveDest | RHIResourceState::ResolveSource)) != RHIResourceState::Common)      
                flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

            if ((state & RHIResourceState::IndirectArgument) != RHIResourceState::Common)   
                flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

            // Present usually happens at the end of all commands
            if ((state & RHIResourceState::Present) != RHIResourceState::Common)
                flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

            return (flags == 0) ? VK_PIPELINE_STAGE_ALL_COMMANDS_BIT : flags;
        }

        // Inferred AspectMask
        VkImageAspectFlags GetAspectMask(RHIResourceState stateBefore, RHIResourceState stateAfter)
        {
            // As long as it involves Depth reading and writing, it's a Depth resource.
            if ((stateBefore & (RHIResourceState::DepthWrite | RHIResourceState::DepthRead)) != RHIResourceState::Common ||
                (stateAfter & (RHIResourceState::DepthWrite | RHIResourceState::DepthRead)) != RHIResourceState::Common)
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; // If templates are supported

            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    void CommandListVulKan::ResourceBarrier(uint32_t numBarriers, const BarrierDesc* pBarriers)
    {
        if (numBarriers == 0 || pBarriers == nullptr)
            return;
        
        // TODO: 实现资源屏障
    }
}
