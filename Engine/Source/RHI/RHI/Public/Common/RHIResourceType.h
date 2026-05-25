#pragma once
#include <cstdint>
#include <Tools/EnumClassFlags.h>

#define MAX_uint32_t 0xFFFFFFFF
#define MAX_uint8_t 0xFF

namespace RHI
{
/* forward declaration */
class RHIResource;

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

/* Feature Level Type */
enum class FeatureLevel : uint32_t
{
	// DirectX 9 Level
	Level_9_1 = 0x9100,   // Shader Model 2.0
	Level_9_2 = 0x9200,   // Shader Model 2.0+ 
	Level_9_3 = 0x9300,   // Shader Model 3.0
	// DirectX 10 Level
	Level_10_0 = 0xa000,  // Shader Model 4.0
	Level_10_1 = 0xa100,  // Shader Model 4.1
	// DirectX 11 Level
	Level_11_0 = 0xb000,  // Shader Model 5.0
	Level_11_1 = 0xb100,  // Shader Model 5.0 + More Features
	// DirectX 12 Level
	Level_12_0 = 0xc000,  // Shader Model 5.1
	Level_12_1 = 0xc100,  // Shader Model 5.1 + Rasterizer Order Independent
	Level_12_2 = 0xc200   // Shader Model 6.0 + Ray Tracing + Mesh Shaders
	// More ...
};

inline const char* GetFeatureLevelName(FeatureLevel level)
{
	switch (level)
	{
	case FeatureLevel::Level_9_1:  return "DirectX 9.1";
	case FeatureLevel::Level_9_2:  return "DirectX 9.2";
	case FeatureLevel::Level_9_3:  return "DirectX 9.3";
	case FeatureLevel::Level_10_0: return "DirectX 10.0";
	case FeatureLevel::Level_10_1: return "DirectX 10.1";
	case FeatureLevel::Level_11_0: return "DirectX 11.0";
	case FeatureLevel::Level_11_1: return "DirectX 11.1";
	case FeatureLevel::Level_12_0: return "DirectX 12.0";
	case FeatureLevel::Level_12_1: return "DirectX 12.1";
	case FeatureLevel::Level_12_2: return "DirectX 12.2 (Ultimate)";
	// More ...
	default: return "Unknown";
	}
}

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

// 资源屏障类型
enum class ResourceBarrierType : uint8_t
{
	Transition,  // 状态转换屏障
	Aliasing,    // 资源别名屏障
	UAV          // UAV屏障
};

// 资源屏障标志
enum class ResourceBarrierFlags : uint8_t
{
	None       = 0,
	BeginOnly  = 0x1,  // 分割屏障开始
	EndOnly    = 0x2   // 分割屏障结束
};

// 过渡屏障描述
struct ResourceTransitionBarrier
{
	class RHIResource* pResource;     // 资源指针
	uint32_t           Subresource;   // 子资源索引（-1表示所有）
	uint64_t           StateBefore;   // 转换前状态（平台相关）
	uint64_t           StateAfter;    // 转换后状态（平台相关）
};

// 别名屏障描述
struct ResourceAliasingBarrier
{
	class RHIResource* pResourceBefore;  // 切换前资源
	class RHIResource* pResourceAfter;   // 切换后资源
};

// UAV屏障描述
struct ResourceUAVBarrier
{
	class RHIResource* pResource;  // UAV资源（可为nullptr）
};

// 资源屏障描述
struct BarrierDesc
{
	ResourceBarrierType   Type;
	ResourceBarrierFlags  Flags;
	union
	{
		ResourceTransitionBarrier Transition;
		ResourceAliasingBarrier   Aliasing;
		ResourceUAVBarrier        UAV;
	};
};
ENUM_CLASS_FLAGS(ResourceBarrierFlags);

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

// Shader Model Level
enum class ShaderModelVersion
{
    SM_5_0 = 50,    // 5.0
    SM_5_1 = 51,    // 5.1
    SM_6_0 = 60,    // 6.0
    SM_6_1 = 61,    // 6.1
    SM_6_2 = 62,    // 6.2
    SM_6_3 = 63,    // 6.3
    SM_6_4 = 64,    // 6.4
    SM_6_5 = 65,    // 6.5
    SM_6_6 = 66,    // 6.6
    SM_6_7 = 67,    // 6.7
    SM_6_8 = 68,    // 6.8
    SM_6_9 = 69,    // 6.9
    Unknown = 0
};

inline const char* ShaderModelToString(ShaderModelVersion version)
{
    switch (version)
    {
    case ShaderModelVersion::SM_5_0: return "5_0";
    case ShaderModelVersion::SM_5_1: return "5_1";
    case ShaderModelVersion::SM_6_0: return "6_0";
    case ShaderModelVersion::SM_6_1: return "6_1";
    case ShaderModelVersion::SM_6_2: return "6_2";
    case ShaderModelVersion::SM_6_3: return "6_3";
    case ShaderModelVersion::SM_6_4: return "6_4";
    case ShaderModelVersion::SM_6_5: return "6_5";
    case ShaderModelVersion::SM_6_6: return "6_6";
    case ShaderModelVersion::SM_6_7: return "6_7";
    case ShaderModelVersion::SM_6_8: return "6_8";
    case ShaderModelVersion::SM_6_9: return "6_9";
    default: return "Unknown";
    }
}

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
	None = 0, // 无特殊标志 - 默认

	// ===== 管线状态控制 =====
	AllowInputAssemblerInputLayout = 0x1,
	AllowStreamOutput = 0x2,

	// ===== 按阶段拒绝访问 - 默认所有阶段都可访问 =====
    DenyVertexShaderRootAccess       = 0x4,    // 禁止 VS 
    DenyHullShaderRootAccess         = 0x8,    // 禁止 HS
    DenyDomainShaderRootAccess       = 0x10,   // 禁止 DS
    DenyGeometryShaderRootAccess     = 0x20,   // 禁止 GS
    DenyPixelShaderRootAccess        = 0x40,   // 禁止 PS
    DenyComputeShaderRootAccess      = 0x200,  // 禁止 CS

	// ===== 新特性：Mesh Shading 管线 =====
	AllowAmplificationShaderRootAccess  = 0x80, // 允许 AM（放大着色器）
	AllowMeshShaderRootAccess        	= 0x100, // 允许 MS（网格着色器）
};
ENUM_CLASS_FLAGS(RootSignatureFlags);

enum class ShaderVisibility : uint32_t
{
    None        = 0,

    // 基础阶段位
    VertexBit   = 1u << 0,   // 0x1  - 顶点着色器
    HullBit     = 1u << 1,   // 0x2  - 外壳着色器
    DomainBit   = 1u << 2,   // 0x4  - 域着色器
    GeometryBit = 1u << 3,   // 0x8  - 几何着色器
    PixelBit    = 1u << 4,   // 0x10 - 像素着色器
    AmpBit      = 1u << 5,   // 0x20 - 放大着色器 / Task
    MeshBit     = 1u << 6,   // 0x40 - 网格着色器
    ComputeBit  = 1u << 7,   // 0x80 - 计算着色器

    // 常用预设组合
    AllGraphics = VertexBit | HullBit | DomainBit | GeometryBit | PixelBit | AmpBit | MeshBit,
    All         = AllGraphics | ComputeBit,
    VertexPixel = VertexBit | PixelBit,
    VertexPixelGeometry = VertexBit | PixelBit | GeometryBit,
};
ENUM_CLASS_FLAGS(ShaderVisibility);
}
