#pragma once
#include <cstdint>
#include <Tools/EnumClassFlags.h>

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
	RRT_ConstantBufferView,
	RRT_UnorderedAccessView,
	RRT_ShaderResourceView,
	RRT_DepthStencilView,
	RRT_RenderTargetView,
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

/* Command Type */
enum class RHICmdType : uint8_t
{
	Graphics,
	Compute,
	Copy
};

// RHI 抽象层的纹理格式枚举
enum class RHITextureFormat : uint8_t
{
	Unknown = 0,
	// 8-bit per component
	R8_UNORM,
	R8_SNORM,
	R8_UINT,
	R8_SINT,
	R8_SRGB,
	// 16-bit per component
	R16_UNORM,
	R16_SNORM,
	R16_UINT,
	R16_SINT,
	R16_FLOAT,
	// 8-bit 2 components
	R8G8_UNORM,
	R8G8_SNORM,
	R8G8_UINT,
	R8G8_SINT,
	R8G8_SRGB,
	// 16-bit 2 components
	R16G16_UNORM,
	R16G16_SNORM,
	R16G16_UINT,
	R16G16_SINT,
	R16G16_FLOAT,
	// 32-bit 2 components
	R32G32_UINT,
	R32G32_SINT,
	R32G32_FLOAT,
	// 8-bit 4 components
	R8G8B8A8_UNORM,
	R8G8B8A8_SNORM,
	R8G8B8A8_UINT,
	R8G8B8A8_SINT,
	R8G8B8A8_SRGB,
	// 16-bit 4 components
	R16G16B16A16_UNORM,
	R16G16B16A16_SNORM,
	R16G16B16A16_UINT,
	R16G16B16A16_SINT,
	R16G16B16A16_FLOAT,
	// 32-bit 3 components
    R32G32B32_UINT,
    R32G32B32_SINT,
    R32G32B32_FLOAT, 
	// 32-bit 4 components
	R32G32B32A32_UINT,
	R32G32B32A32_SINT,
	R32G32B32A32_FLOAT,
	// 10:10:10:2 packed
	R10G10B10A2_UNORM,
	R10G10B10A2_UINT,
	// 11:11:10 packed
	R11G11B10_FLOAT,
	// 5:6:5 packed
	B5G6R5_UNORM,
	// 5:5:5:1 packed
	B5G5R5A1_UNORM,
	// 4:4:4:4 packed
	B4G4R4A4_UNORM,
	// BGRA formats
	B8G8R8A8_UNORM,
	B8G8R8A8_SRGB,
	B8G8R8X8_UNORM,
	B8G8R8X8_SRGB,
	// Depth formats
	D16_UNORM,
	D24_UNORM_S8_UINT,
	D32_FLOAT,
	D32_FLOAT_S8X24_UINT,
	// Compressed formats (BC)
	BC1_UNORM,
	BC1_UNORM_SRGB,
	BC2_UNORM,
	BC2_UNORM_SRGB,
	BC3_UNORM,
	BC3_UNORM_SRGB,
	BC4_UNORM,
	BC4_SNORM,
	BC5_UNORM,
	BC5_SNORM,
	BC6H_UF16,
	BC6H_SF16,
	BC7_UNORM,
	BC7_UNORM_SRGB,
	// Typeless (for creating views with different formats)
	R8G8B8A8_TYPELESS,
	R16G16B16A16_TYPELESS,
	R32G32B32A32_TYPELESS,
	R32_TYPELESS,
	
	Max
};

// 判断格式是否有深度/模板分量
inline bool IsDepthFormat(RHITextureFormat Format)
{
	switch (Format)
	{
	case RHITextureFormat::D16_UNORM:
	case RHITextureFormat::D24_UNORM_S8_UINT:
	case RHITextureFormat::D32_FLOAT:
	case RHITextureFormat::D32_FLOAT_S8X24_UINT:
		return true;
	default:
		return false;
	}
}

// 判断格式是否有模板分量
inline bool IsStencilFormat(RHITextureFormat Format)
{
	switch (Format)
	{
	case RHITextureFormat::D24_UNORM_S8_UINT:
	case RHITextureFormat::D32_FLOAT_S8X24_UINT:
		return true;
	default:
		return false;
	}
}

// 判断格式是否压缩
inline bool IsCompressedFormat(RHITextureFormat Format)
{
	switch (Format)
	{
	case RHITextureFormat::BC1_UNORM:
	case RHITextureFormat::BC1_UNORM_SRGB:
	case RHITextureFormat::BC2_UNORM:
	case RHITextureFormat::BC2_UNORM_SRGB:
	case RHITextureFormat::BC3_UNORM:
	case RHITextureFormat::BC3_UNORM_SRGB:
	case RHITextureFormat::BC4_UNORM:
	case RHITextureFormat::BC4_SNORM:
	case RHITextureFormat::BC5_UNORM:
	case RHITextureFormat::BC5_SNORM:
	case RHITextureFormat::BC6H_UF16:
	case RHITextureFormat::BC6H_SF16:
	case RHITextureFormat::BC7_UNORM:
	case RHITextureFormat::BC7_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

enum class RHIResourceState : uint64_t
{
	// 基础状态
    Common                      	= 0,
    VertexBuffer                	= 0x1,
    IndexBuffer                		= 0x2,
    RenderTarget                	= 0x4,
    UnorderedAccess             	= 0x8,
    DepthWrite                  	= 0x10,
    DepthRead                   	= 0x20,
    NonPixelShaderResource      	= 0x40,
    PixelShaderResource         	= 0x80,
    StreamOut                   	= 0x100,
    IndirectArgument            	= 0x200,
    CopyDest                    	= 0x400,
    CopySource                  	= 0x800,
    ResolveDest                 	= 0x1000,
    ResolveSource               	= 0x2000,
    // 视频相关	
    VideoDecodeRead             	= 0x10000,
    VideoDecodeWrite            	= 0x20000,
    VideoProcessRead            	= 0x40000,
    VideoProcessWrite           	= 0x80000,
    VideoEncodeRead             	= 0x200000,
    VideoEncodeWrite            	= 0x800000,
    // 光线追踪
    RaytracingAccelerationStructure = 0x400000,
    // 着色率
    ShadingRateSource             	= 0x1000000,
    // 组合状态
    Present                     	= 0,
    GenericRead                 	= VertexBuffer | IndexBuffer | NonPixelShaderResource | 
                  						PixelShaderResource | IndirectArgument | CopySource | ResolveSource,
    AllShaderResource           	= NonPixelShaderResource | PixelShaderResource,
};
ENUM_CLASS_FLAGS(RHIResourceState);

enum class BufferBindFlag : uint32_t
{
	None           = 0,			 // 无绑定标志
	VertexBuffer   = 0x1 << 0,   // 顶点缓冲区
	IndexBuffer    = 0x1 << 1,   // 索引缓冲区
	ConstantBuffer = 0x1 << 2,   // 常量缓冲区
	ShaderResource = 0x1 << 3,   // 着色器资源视图 (SRV)
	UnorderedAccess= 0x1 << 4,   // 无序访问视图 (UAV)
	RenderTarget   = 0x1 << 5,   // 渲染目标
	DepthStencil   = 0x1 << 6,   // 深度模板
	StreamOutput   = 0x1 << 7,   // 流输出
};
ENUM_CLASS_FLAGS(BufferBindFlag);

enum class RHIPrimitiveTopology : uint8_t
{
	Undefined,
	PointList,
	LineList,
	LineStrip,
	TriangleList,
	TriangleStrip,
	LineListAdj,
	LineStripAdj,
	TriangleListAdj,
	TriangleStripAdj,
	ControlPointPatchList
};

enum class RHIIndexFormat : uint8_t
{
	Uint16,
	Uint32
};
	
enum class InputClassification : uint8_t
{
	PerVertexData   = 0,  // 每顶点数据
	PerInstanceData = 1   // 每实例数据
};

struct RHIRect
{
	float left;
	float top;
	float right;
	float bottom;
};

struct RHIViewport
{
	float topLeftX;
	float topLeftY;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};

enum class RHIClearFlags : uint8_t
{
    None = 0,
    Depth = 1 << 0,      // 0b0001 (1)
    Stencil = 1 << 1,    // 0b0010 (2)
    DepthStencil = Depth | Stencil // 0b0011 (3)
};
ENUM_CLASS_FLAGS(RHIClearFlags);

enum class ShaderType : uint8_t
{
	Vertex,
	Pixel,
	Geometry,
	Hull,
	Domain,
	Compute
	// More ...
};

/*
* 根参数类型
* 一般跟签名就是描述这个Shader的资源布局信息，包括描述符表、常量缓冲区、CBV、SRV、UAV等。
* 描述符表就是一个类似的一维Range(描述符范围)数组，首先它可以有多个Range 
*，每个Range声明了对应类型的堆的起始的X到N的所有描述符（也就是说，每个Range由一个起始地址与长度构成），
* 多个Range也就可以有多个类型，他们构成了描述符表。
* CBV、SRV、UAV等都是单个描述符，类型数为1，并且也只能有一个资源地址。
* Constants（比如一个float）是直接把数据写在根签名中的。
*/
enum class RootParameterType : uint8_t
{
	DescriptorTable,
	Constants,
	CBV,
	SRV,
	UAV
};

// 描述符表类型 - 注意这里的采样器只能是在描述符表中使用
enum class DescriptorRangeType : uint32_t
{
    SRV     = 0,  // 着色器资源视图（纹理、结构化缓冲区等） - 只读
    UAV     = 1,  // 无序访问视图 - 可读写
    CBV     = 2,  // 常量缓冲区视图
    Sampler = 3,  // 采样器（纹理采样状态）
};

enum class RootSignatureFlags : uint32_t
{
	None = 0, // None - default

	// ===== Pipeline state control =====
	AllowInputAssemblerInputLayout = 0x1,
	AllowStreamOutput = 0x2,

	// ===== Stage access control =====
    DenyVertexShaderRootAccess       = 0x4,    // 禁止 VS 
    DenyHullShaderRootAccess         = 0x8,    // 禁止 HS
    DenyDomainShaderRootAccess       = 0x10,   // 禁止 DS
    DenyGeometryShaderRootAccess     = 0x20,   // 禁止 GS
    DenyPixelShaderRootAccess        = 0x40,   // 禁止 PS
    DenyComputeShaderRootAccess      = 0x200,  // 禁止 CS

	// ===== Mesh Shading pipeline =====
	AllowAmplificationShaderRootAccess  = 0x80, // 允许 AM（放大着色器）
	AllowMeshShaderRootAccess        	= 0x100, // 允许 MS（网格着色器）
};
ENUM_CLASS_FLAGS(RootSignatureFlags);

enum class ShaderVisibility : uint8_t
{
    None        = 0,    // 无
    All         = 1,    // 所有阶段可见
    Vertex      = 2,    // 仅顶点着色器
    Hull        = 3,    // 仅外壳着色器
    Domain      = 4,    // 仅域着色器
    Geometry    = 5,    // 仅几何着色器
    Pixel       = 6,    // 仅像素着色器
    Amplification = 7,  // 仅放大着色器
    Mesh        = 8,    // 仅网格着色器
    Compute     = 9,    // 仅计算着色器
};
}
