#include "RHICommandListVulKan.h"

namespace RHI
{
    namespace
    {
        // Helper: determine image aspect mask from RHITextureFormat
        VkImageAspectFlags GetImageAspectMask(RHITextureFormat format)
        {
            switch (format)
            {
            case RHITextureFormat::D16_UNORM:
            case RHITextureFormat::D32_FLOAT:
                return VK_IMAGE_ASPECT_DEPTH_BIT;
            case RHITextureFormat::D24_UNORM_S8_UINT:
            case RHITextureFormat::D32_FLOAT_S8X24_UINT:
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }
    }

    void CommandListVulKan::CopyResource(RHIResource* pDstResource, RHIResource* pSrcResource)
    {
        if (!pDstResource || !pSrcResource)
            return;

        RHIResourceType dstType = pDstResource->GetType();
        RHIResourceType srcType = pSrcResource->GetType();

        // ------------------------------------------------------------
        // Buffer -> Buffer
        // ------------------------------------------------------------
        if (dstType == RRT_Buffer && srcType == RRT_Buffer)
        {
            BufferVulKan* dstBuffer = static_cast<BufferVulKan*>(pDstResource);
            BufferVulKan* srcBuffer = static_cast<BufferVulKan*>(pSrcResource);

            VkDeviceSize dstSize = (VkDeviceSize)dstBuffer->GetSize();
            VkDeviceSize srcSize = (VkDeviceSize)srcBuffer->GetSize();
            VkDeviceSize copySize = dstSize < srcSize ? dstSize : srcSize;

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = copySize;

            vkCmdCopyBuffer(
                m_CommandBuffer,
                srcBuffer->GetBuffer(),
                dstBuffer->GetBuffer(),
                1, &copyRegion);
            return;
        }

        // ------------------------------------------------------------
        // Texture -> Texture
        // ------------------------------------------------------------
        if (dstType == RRT_Texture && srcType == RRT_Texture)
        {
            TextureVulKan* dstTex = static_cast<TextureVulKan*>(pDstResource);
            TextureVulKan* srcTex = static_cast<TextureVulKan*>(pSrcResource);

            const TextureDesc& dstDesc = dstTex->GetDesc();
            const TextureDesc& srcDesc = srcTex->GetDesc();

            VkImageAspectFlags aspectMask = GetImageAspectMask(srcDesc.Format);

            uint32_t mipLevels   = dstDesc.MipLevels   < srcDesc.MipLevels   ? dstDesc.MipLevels   : srcDesc.MipLevels;
            uint32_t arrayLayers = dstDesc.ArrayLayers < srcDesc.ArrayLayers ? dstDesc.ArrayLayers : srcDesc.ArrayLayers;
            uint32_t width       = dstDesc.Width       < srcDesc.Width       ? dstDesc.Width       : srcDesc.Width;
            uint32_t height      = dstDesc.Height      < srcDesc.Height      ? dstDesc.Height      : srcDesc.Height;
            uint32_t depth       = dstDesc.Depth       < srcDesc.Depth       ? dstDesc.Depth       : srcDesc.Depth;

            VkImageCopy copyRegion{};
            copyRegion.srcSubresource.aspectMask = aspectMask;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.layerCount = arrayLayers;
            copyRegion.srcOffset = { 0, 0, 0 };

            copyRegion.dstSubresource.aspectMask = aspectMask;
            copyRegion.dstSubresource.mipLevel = 0;
            copyRegion.dstSubresource.baseArrayLayer = 0;
            copyRegion.dstSubresource.layerCount = arrayLayers;
            copyRegion.dstOffset = { 0, 0, 0 };

            copyRegion.extent = { width, height, depth };

            // Note: assumes caller transitioned layouts to
            //       TRANSFER_SRC_OPTIMAL / TRANSFER_DST_OPTIMAL appropriately.
            vkCmdCopyImage(
                m_CommandBuffer,
                srcTex->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstTex->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &copyRegion);
            return;
        }

        // ------------------------------------------------------------
        // Buffer -> Texture
        // ------------------------------------------------------------
        if (dstType == RRT_Texture && srcType == RRT_Buffer)
        {
            TextureVulKan* dstTex = static_cast<TextureVulKan*>(pDstResource);
            BufferVulKan*  srcBuf = static_cast<BufferVulKan*>(pSrcResource);

            const TextureDesc& dstDesc = dstTex->GetDesc();
            VkImageAspectFlags aspectMask = GetImageAspectMask(dstDesc.Format);

            VkBufferImageCopy copyRegion{};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;     // tightly packed
            copyRegion.bufferImageHeight = 0;   // tightly packed

            copyRegion.imageSubresource.aspectMask = aspectMask;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = dstDesc.ArrayLayers;

            copyRegion.imageOffset = { 0, 0, 0 };
            copyRegion.imageExtent = { dstDesc.Width, dstDesc.Height, dstDesc.Depth };

            vkCmdCopyBufferToImage(
                m_CommandBuffer,
                srcBuf->GetBuffer(),
                dstTex->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &copyRegion);
            return;
        }

        // ------------------------------------------------------------
        // Texture -> Buffer
        // ------------------------------------------------------------
        if (dstType == RRT_Buffer && srcType == RRT_Texture)
        {
            BufferVulKan*  dstBuf = static_cast<BufferVulKan*>(pDstResource);
            TextureVulKan* srcTex = static_cast<TextureVulKan*>(pSrcResource);

            const TextureDesc& srcDesc = srcTex->GetDesc();
            VkImageAspectFlags aspectMask = GetImageAspectMask(srcDesc.Format);

            VkBufferImageCopy copyRegion{};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;

            copyRegion.imageSubresource.aspectMask = aspectMask;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = srcDesc.ArrayLayers;

            copyRegion.imageOffset = { 0, 0, 0 };
            copyRegion.imageExtent = { srcDesc.Width, srcDesc.Height, srcDesc.Depth };

            vkCmdCopyImageToBuffer(
                m_CommandBuffer,
                srcTex->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstBuf->GetBuffer(),
                1, &copyRegion);
            return;
        }

        ThrowErrorMessage("CommandListVulKan::CopyResource: Unsupported resource type combination");
    }

    void CommandListVulKan::CopyBufferRegion(RHIBuffer* pDstBuffer, uint64_t dstOffset, RHIBuffer* pSrcBuffer, uint64_t srcOffset, uint64_t numBytes)
    {
        if (!pDstBuffer || !pSrcBuffer || numBytes == 0)
            return;

        BufferVulKan* dxDstBuffer = static_cast<BufferVulKan*>(pDstBuffer);
        BufferVulKan* dxSrcBuffer = static_cast<BufferVulKan*>(pSrcBuffer);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = (VkDeviceSize)srcOffset;
        copyRegion.dstOffset = (VkDeviceSize)dstOffset;
        copyRegion.size      = (VkDeviceSize)numBytes;

        vkCmdCopyBuffer(
            m_CommandBuffer,
            dxSrcBuffer->GetBuffer(),
            dxDstBuffer->GetBuffer(),
            1, &copyRegion);
    }
}
