#pragma once
#include <cstdint>

#define MAX_uint32_t 0xFFFFFFFF
#define MAX_uint8_t 0xFF

namespace RHI
{
/** An enumeration of the different RHI reference types. */
enum RHIResourceType : uint8_t
{
	RRT_None,

	RRT_SamplerState,
	RRT_RasterizerState,
	RRT_DepthStencilState,
	RRT_BlendState,
	RRT_VertexDeclaration,
	RRT_VertexShader,
	RRT_MeshShader,
	RRT_AmplificationShader,
	RRT_PixelShader,
	RRT_GeometryShader,
	RRT_RayTracingShader,
	RRT_ComputeShader,
	RRT_GraphicsPipelineState,
	RRT_ComputePipelineState,
	RRT_RayTracingPipelineState,
	RRT_BoundShaderState,
	RRT_UniformBufferLayout,
	RRT_UniformBuffer,
	RRT_Buffer,
	RRT_Texture,
	RRT_TextureReference,
	RRT_TimestampCalibrationQuery,
	RRT_GPUFence,
	RRT_RenderQuery,
	RRT_RenderQueryPool,
	RRT_Viewport,
	RRT_UnorderedAccessView,
	RRT_ShaderResourceView,
	RRT_RayTracingAccelerationStructure,
	RRT_RayTracingShaderBindingTable,
	RRT_StagingBuffer,
	RRT_CustomPresent,
	RRT_ShaderLibrary,
	RRT_ShaderBundle,
	RRT_WorkGraphShader,
	RRT_WorkGraphPipelineState,
	RRT_StreamSourceSlot,
	RRT_ResourceCollection,

	RRT_Num
};

enum class RHIDescriptorHeapType : uint8_t
{
	Standard,
	Sampler,
	RenderTarget,
	DepthStencil,
	Count,
	Invalid = MAX_uint8_t
};

enum class SamplerFilter : uint8_t
{
	Point,
	Bilinear,
	Trilinear,
	Anisotropic
};

enum class SamplerAddressMode : uint8_t
{
	Wrap,
	Mirror,
	Clamp,
	Border,
	MirrorOnce
};

enum class SamplerComparisonFunc : uint8_t
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always
};

/**
* 缓冲区堆类型
* 决定缓冲区的内存位置和 CPU/GPU 访问权限
*/
enum class BufferHeapType : uint8_t
{
	/**
	* Default 堆（GPU 本地显存）
	* 
	* CPU 访问：不可访问
	* GPU 访问：读写
	*/
	Default,
	
	/**
	* Upload 堆（CPU 上传）
	* 
	* CPU 访问：可写入
	* GPU 访问：可读取
	*/
	Upload,
	
	/**
	* Readback 堆（CPU 回读）
	* 
	* CPU 访问：可读写
	* GPU 访问：只写（或作为拷贝目标）
	*/
	Readback
};

/* Command List Type */
enum class RHICmdListType : uint8_t
{
	Graphics,
	Compute,
	Copy
};

}
