#pragma once
#include "RHITypes.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace RHI
{

// Swap Chain Description
struct SwapChainDesc
{
    void* WindowHandle;
    uint32_t Width;
    uint32_t Height;
    bool VSync = false;
};

// Resource Transition Barrier Description
struct ResourceTransitionBarrier
{
	class RHIResource* 			pResource;     // Resource Pointer
	uint32_t           			Subresource;   // Subresource Index (-1 for all)
	RHIResourceState            StateBefore;   // State Before Transition
	RHIResourceState            StateAfter;    // State After Transition
};

// Resource Aliasing Barrier Description
struct ResourceAliasingBarrier
{
	class RHIResource* pResourceBefore;  // Resource Before Transition
	class RHIResource* pResourceAfter;   // Resource After Transition
};

// UAV Barrier Description
struct ResourceUAVBarrier
{
	class RHIResource* pResource;  // UAV Resource（can be nullptr）
};

// Resource Barrier Description
struct BarrierDesc
{
	ResourceBarrierType   Type 			    = ResourceBarrierType::Transition;
	BarrierResourceType   ResourceType 		= BarrierResourceType::Buffer;
	ResourceBarrierFlags  Flags 			= ResourceBarrierFlags::None;
	union
	{
		ResourceTransitionBarrier Transition;
		ResourceAliasingBarrier   Aliasing;
		ResourceUAVBarrier        UAV;
	};
};

struct TextureDesc
{
    RHITextureFormat Format;
    uint32_t Width;
    uint32_t Height;
    uint32_t Depth;
    uint32_t MipLevels;
    uint32_t ArrayLayers;
    uint32_t SampleCount;
    uint32_t SampleQuality;
    uint32_t Flags;
};

/*
 * @brief SPIR-V Resource Description
 * This struct is used to describe the data layout in a buffer
*/
struct SPIRVResource {
    std::string name;           // Resource Name
    uint32_t binding;           // binding Index (corresponding HLSL register)
    uint32_t set;               // set Index (corresponding HLSL space)
    uint32_t size;              // Resource Size（bytes）
    bool isConstantBuffer;      // Is Constant Buffer Resource
    bool isTexture;             // Is Texture Resource
    bool isSampler;             // Is Sampler Resource
    std::vector<std::pair<std::string, uint32_t>> members; // Member Variables
};

/*
 * @brief SPIR-V Reflection Information
 * This struct is used to store the reflection information of SPIR-V
*/
struct SPIRVReflection {
    std::string entryPoint;
    std::vector<SPIRVResource> resources;
    std::unordered_map<uint32_t, std::vector<SPIRVResource>> resourcesBySet; // Group resources by set (space)
    uint32_t totalConstantBufferSize = 0;
};

/*
 * @brief Shader Macro Description
 * This struct is used to describe the shader macro
*/
struct ShaderMacro
{
    // Macro Name
    std::string name;
    // Macro Definition
    std::string definition;
};

/*
 * @brief Shader Compile Options
 * This struct is used to store the compile options of shader
*/
struct ShaderCompileOptions {
    std::string entryPoint = "main";
    std::string targetProfile = "vs_6_0";  // vs_6_0, ps_6_0, cs_6_0
    // Whether to optimize the shader
    bool optimize = true;
    // Whether to enable debug info
    bool debugInfo = false;
    // Whether to enable 16-bit types - Reduce precision to save bandwidth
    bool enable16BitTypes = false;
    // Whether to enable matrix packing
    bool enableMatrixPacking = false;

    // Macros to define
    std::vector<ShaderMacro> Macros;
    // Include Paths - Add some custom paths to include files from
    std::vector<std::string> includePaths;
    // HLSL Version
    static constexpr const char* DEFAULT_HLSL_VERSION = "2021";
};

// Shader Compile Result
struct ShaderCompileResult {
    bool success = false;           // compile success
    std::vector<uint32_t> byteCode;    // shader byte code
    std::string errorMessage;       // error msg
    std::string warningMessage;     // warning msg
    size_t bytecodeSize = 0;        // bytecode size
};

// Shader Compile Description
struct ShaderCompileDesc
{
    ShaderType Type;
    std::vector<ShaderMacro> Macros = {};
    const char* SourceCode = nullptr;
    const char* EntryPoint = "main";
	// For regular shaders, we don't want you to specify a profile,
	// because the support for Shader Models varies across different platforms.
	//  Our shader compiler will automatically choose the appropriate profile based on the platform.
    const char* Profile = nullptr;
    const char* FilePath = nullptr;
    bool EnableDebugInfo = false;
};
	
// Input Element Description
struct InputElementDesc
{
	const char* SemanticName;           		  // Semantic Name
	uint32_t SemanticIndex;              		  // Semantic Index
	RHITextureFormat Format;                      // Data Format
	uint32_t InputSlot;              			  // Input Slot Index（0-15）
	uint32_t AlignedByteOffset;              	  // Aligned Byte Offset（use APPDEND_ALIGNED_ELEMENT to calculate）
	InputClassification InputSlotClass;           // Input Classification
	uint32_t InstanceDataStepRate;                // Instance Data Step Rate
	// Auxiliary Constants
	static constexpr uint32_t APPEND_ALIGNED_ELEMENT = 0xFFFFFFFF;
};
}

