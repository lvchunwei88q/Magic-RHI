#pragma once

#include <vulkan.h>
#include <Common/Check.h>
#include <RHIResource.h>
#include <vector>

namespace RHI
{
    // ==================================================== State
    class RasterizerStateVulKan : public RHIRasterizerState
    {
    public:
        RasterizerStateVulKan(const VkPipelineRasterizationStateCreateInfo& info)
            : m_RasterizationInfo(info) {}
        ~RasterizerStateVulKan() override = default;

        const VkPipelineRasterizationStateCreateInfo& GetRasterizationInfo() const { return m_RasterizationInfo; }

    private:
        VkPipelineRasterizationStateCreateInfo m_RasterizationInfo;
    };

    class BlendStateVulKan : public RHIBlendState
    {
    public:
        BlendStateVulKan(const VkPipelineColorBlendStateCreateInfo& info)
            : m_BlendInfo(info) {}
        ~BlendStateVulKan() override = default;

        const VkPipelineColorBlendStateCreateInfo& GetBlendInfo() const { return m_BlendInfo; }

    private:
        VkPipelineColorBlendStateCreateInfo m_BlendInfo;
    };

    class DepthStencilStateVulKan : public RHIDepthStencilState
    {
    public:
        DepthStencilStateVulKan(const VkPipelineDepthStencilStateCreateInfo& info)
            : m_DepthStencilInfo(info) {}
        ~DepthStencilStateVulKan() override = default;

        const VkPipelineDepthStencilStateCreateInfo& GetDepthStencilInfo() const { return m_DepthStencilInfo; }

    private:
        VkPipelineDepthStencilStateCreateInfo m_DepthStencilInfo;
    };

    class SamplerStateVulKan : public RHISamplerState
    {
    public:
        SamplerStateVulKan(VkSampler sampler, RHIDescriptorHandle handle, const VkDevice* device)
            : m_Sampler(sampler)
            , m_Device(device)
            , RHISamplerState(handle) {}
        ~SamplerStateVulKan() override;

        VkSampler GetSampler() const { return m_Sampler; }
        VkDevice GetDevice() const { return *m_Device; }

    private:
        VkSampler m_Sampler;
        const VkDevice* m_Device;
    };

    // ==================================================== Buffer
    class BufferVulKan : public RHIBuffer
    {
    public:
        BufferVulKan(VkBuffer buffer, VkDeviceMemory memory, const BufferDesc& desc, const VkDevice* device)
            : RHIBuffer(desc, RRT_Buffer)
            , m_Buffer(buffer)
            , m_Memory(memory)
            , m_Device(device)
        {
        }

        ~BufferVulKan() override;

        VkBuffer GetBuffer() const { return m_Buffer; }
        VkDeviceMemory GetMemory() const { return m_Memory; }

        void* Map() override;
        void Unmap() override;

        uint64_t GetGPUVirtualAddress() const;

    private:
        VkDevice GetDevice() const { return *m_Device; }
    private:
        VkBuffer m_Buffer;
        VkDeviceMemory m_Memory;
        const VkDevice* m_Device;
        void* m_MappedData = nullptr;
    };

    class TextureVulKan : public RHITexture
    {
    public:
        TextureVulKan(VkImage image, VkDeviceMemory memory, const TextureDesc& desc, const VkDevice* device)
            : RHITexture(desc)
            , m_Image(image)
            , m_Memory(memory)
            , m_Device(device) {}
        ~TextureVulKan() override;

        uint64_t GetSize() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;

        VkImage GetImage() const { return m_Image; }
        VkDeviceMemory GetMemory() const { return m_Memory; }

    private:
        VkDevice GetDevice() const { return *m_Device; }
    private:
        VkImage m_Image;
        VkDeviceMemory m_Memory;
        const VkDevice* m_Device;
    };

    // ==================================================== View
    class ConstantBufferViewVulKan : public RHIConstantBufferView
    {
    public:
        ConstantBufferViewVulKan(VkBuffer buffer, uint64_t offset, uint64_t range)
            : m_Buffer(buffer)
            , m_Offset(offset)
            , m_Range(range) {}
        ~ConstantBufferViewVulKan() override = default;

        VkBuffer GetBuffer() const { return m_Buffer; }
        uint64_t GetOffset() const { return m_Offset; }
        uint64_t GetRange() const { return m_Range; }
        uint64_t GetGPUVirtualAddress() const override;

    private:
        VkBuffer m_Buffer;
        uint64_t m_Offset;
        uint64_t m_Range;
    };

    class ShaderResourceViewVulKan : public RHIShaderResourceView
    {
    public:
        ShaderResourceViewVulKan(VkImageView imageView, VkImageLayout layout, const VkDevice* device)
            : m_ImageView(imageView)
            , m_ImageLayout(layout)
            , m_Device(device) {}
        ShaderResourceViewVulKan(VkBuffer buffer, uint64_t offset, uint64_t range, const VkDevice* device)
            : m_Buffer(buffer)
            , m_Offset(offset)
            , m_Range(range)
            , m_Device(device) {}
        ~ShaderResourceViewVulKan() override;

        VkImageView GetImageView() const { return m_ImageView; }
        VkImageLayout GetImageLayout() const { return m_ImageLayout; }
        VkBuffer GetBuffer() const { return m_Buffer; }
        uint64_t GetOffset() const { return m_Offset; }
        uint64_t GetRange() const { return m_Range; }
        
        uint64_t GetGPUVirtualAddress() const override;

    private:
        VkDevice GetDevice() const { return *m_Device; }
    private:
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        uint64_t m_Offset = 0;
        uint64_t m_Range = 0;
        const VkDevice* m_Device;
    };

    class UnorderedAccessViewVulKan : public RHIUnorderedAccessView
    {
    public:
        UnorderedAccessViewVulKan(VkImageView imageView, VkImageLayout layout, const VkDevice* device)
            : m_ImageView(imageView)
            , m_ImageLayout(layout)
            , m_Device(device) {}
        UnorderedAccessViewVulKan(VkBuffer buffer, uint64_t offset, uint64_t range, const VkDevice* device)
            : m_Buffer(buffer)
            , m_Offset(offset)
            , m_Range(range)
            , m_Device(device) {}
        ~UnorderedAccessViewVulKan() override;

        VkImageView GetImageView() const { return m_ImageView; }
        VkImageLayout GetImageLayout() const { return m_ImageLayout; }
        VkBuffer GetBuffer() const { return m_Buffer; }
        uint64_t GetOffset() const { return m_Offset; }
        uint64_t GetRange() const { return m_Range; }
        uint64_t GetGPUVirtualAddress() const override;

    private:
        VkDevice GetDevice() const { return *m_Device; }
    private:
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        uint64_t m_Offset = 0;
        uint64_t m_Range = 0;
        const VkDevice* m_Device;
    };

    class RenderTargetViewVulKan : public RHIRenderTargetView
    {
    public:
        RenderTargetViewVulKan(VkImageView imageView, const VkDevice* device)
            : m_ImageView(imageView)
            , m_Device(device) {}
        ~RenderTargetViewVulKan() override;

        VkImageView GetImageView() const { return m_ImageView; }
        
    private:
        VkDevice GetDevice() const { return *m_Device; }
    private:
        VkImageView m_ImageView;
        const VkDevice* m_Device;
    };

    class DepthStencilViewVulKan : public RHIDepthStencilView
    {
    public:
        DepthStencilViewVulKan(VkImageView imageView, const VkDevice* device)
            : m_ImageView(imageView)
            , m_Device(device) {}
        ~DepthStencilViewVulKan() override;

        VkImageView GetImageView() const { return m_ImageView; }
        
    private:
        VkDevice GetDevice() const { return *m_Device; }
    private:
        VkImageView m_ImageView;
        const VkDevice* m_Device;
    };

    // ==================================================== Shader
    class VertexShaderVulKan : public RHIVertexShader
    {
    public:
        VertexShaderVulKan(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv) {}
        ~VertexShaderVulKan() override = default;
        const std::vector<uint32_t>& GetSPIRV() const { return m_SPIRV; }
    private:
        std::vector<uint32_t> m_SPIRV;
    };

    class PixelShaderVulKan : public RHIPixelShader
    {
    public:
        PixelShaderVulKan(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv) {}
        ~PixelShaderVulKan() override = default;
        const std::vector<uint32_t>& GetSPIRV() const { return m_SPIRV; }
    private:
        std::vector<uint32_t> m_SPIRV;
    };

    class GeometryShaderVulKan : public RHIGeometryShader
    {
    public:
        GeometryShaderVulKan(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv) {}
        ~GeometryShaderVulKan() override = default;
        const std::vector<uint32_t>& GetSPIRV() const { return m_SPIRV; }
    private:
        std::vector<uint32_t> m_SPIRV;
    };

    class HullShaderVulKan : public RHIHullShader
    {
    public:
        HullShaderVulKan(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv) {}
        ~HullShaderVulKan() override = default;
        const std::vector<uint32_t>& GetSPIRV() const { return m_SPIRV; }
    private:
        std::vector<uint32_t> m_SPIRV;
    };

    class DomainShaderVulKan : public RHIDomainShader
    {
    public:
        DomainShaderVulKan(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv) {}
        ~DomainShaderVulKan() override = default;
        const std::vector<uint32_t>& GetSPIRV() const { return m_SPIRV; }
    private:
        std::vector<uint32_t> m_SPIRV;
    };

    class ComputeShaderVulKan : public RHIComputeShader
    {
    public:
        ComputeShaderVulKan(const std::vector<uint32_t>& spirv) : m_SPIRV(spirv) {}
        ~ComputeShaderVulKan() override = default;
        const std::vector<uint32_t>& GetSPIRV() const { return m_SPIRV; }
    private:
        std::vector<uint32_t> m_SPIRV;
    };
}
