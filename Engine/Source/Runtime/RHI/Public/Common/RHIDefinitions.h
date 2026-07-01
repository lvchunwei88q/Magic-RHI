#pragma once
#include "RHITypes.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace RHI
{

// Swap Chain Description
struct SwapChainDesc
{
    void* WindowHandleRef;
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
    std::string targetProfile = "xxx_6_0";  // vs_6_0, ps_6_0, cs_6_0
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

struct ShaderCompileOptionInternal : public ShaderCompileOptions
{
    // Target Compiler mode (SPIR-V or HLSL)
    std::string targetCompilerMode = "-spirv";
    // Target SPIR-V environment (vulkan1.0 or vulkan1.3)
    std::string SPIR_V_TargetEnv = "vulkan1.0";

    ShaderCompileOptionInternal() = default;
    ShaderCompileOptionInternal(const ShaderCompileOptions& options) : ShaderCompileOptions(options) {}
};

// ========================================================== Post-Process Args
struct ShaderPostProcessArgs {
    RHIResourceType ShaderType;

    virtual bool IsValid() const = 0;

    ShaderPostProcessArgs() = default;
    explicit ShaderPostProcessArgs(RHIResourceType type) : ShaderType(type) {}
    virtual ~ShaderPostProcessArgs() = default;
};

struct VertexShaderPostProcessArgs : public ShaderPostProcessArgs
{
    std::vector<InputElementDesc> inputLayout;

    bool IsValid() const override { return inputLayout.size() > 0; }
    VertexShaderPostProcessArgs() : ShaderPostProcessArgs(RHIResourceType::RRT_VertexShader) {}
    VertexShaderPostProcessArgs(const std::vector<InputElementDesc>& inputLayout) : inputLayout(inputLayout), ShaderPostProcessArgs(RHIResourceType::RRT_VertexShader) {}
};
struct PixelShaderPostProcessArgs : public ShaderPostProcessArgs
{
    // NOT
    bool IsValid() const override { return true; }
    PixelShaderPostProcessArgs() : ShaderPostProcessArgs(RHIResourceType::RRT_PixelShader) {};
};
struct ComputeShaderPostProcessArgs : public ShaderPostProcessArgs
{
    // NOT
    bool IsValid() const override { return true; }
    ComputeShaderPostProcessArgs() : ShaderPostProcessArgs(RHIResourceType::RRT_ComputeShader) {};
};
struct GeometryShaderPostProcessArgs : public ShaderPostProcessArgs
{
    // NOT
    bool IsValid() const override { return true; }
    GeometryShaderPostProcessArgs() : ShaderPostProcessArgs(RHIResourceType::RRT_GeometryShader) {};
};
// ...
// ==========================================================

// Shader Compile Result
struct ShaderCompileResult {
    bool success = false;           // compile success
    std::vector<uint8_t> byteCode;    // shader byte code
    std::string errorMessage;       // error msg
    std::string warningMessage;     // warning msg
    size_t bytecodeSize = 0;        // bytecode size
};

// Shader Compile Source Description
struct ShaderCompileSource
{
    enum class SourceType {
        SourcePath = 1,
        SourceCode = 2,
    };
    std::string sourceDescription;
    SourceType sourceType = SourceType::SourcePath;
    std::shared_ptr<ShaderPostProcessArgs> postProcessArgs;
};

// Shader Reflection Generation Mode
struct ShaderReflectionGenerationMode {
    enum class ReflectionGenerationMode {
        Use_InitialCompileCache = 1,
        Use_CompileResultCache = 2,
        Use_SourceCacheCompile = 3,
    };
    ReflectionGenerationMode mode = ReflectionGenerationMode::Use_InitialCompileCache;
};

// Create Shader Description
struct CreateShaderDesc
{   
    std::variant<std::vector<uint8_t>, std::vector<uint32_t>> byteCode;

    const std::vector<uint8_t>& GetUINT8ByteCode() const {
        return std::get<std::vector<uint8_t>>(byteCode);
    }
    
    const std::vector<uint32_t>& GetUINT32ByteCode() const {
        return std::get<std::vector<uint32_t>>(byteCode);
    }
};
}

