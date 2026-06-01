#pragma once
#include "RHITypes.h"

namespace RHI
{
// 交换链描述
struct SwapChainDesc
{
    void* WindowHandle;
    uint32_t Width;
    uint32_t Height;
    bool VSync = false;
};

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
ENUM_CLASS_FLAGS(ResourceBarrierFlags);

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

struct ShaderCompileDesc
{
    ShaderType Type;
    const char* SourceCode = nullptr;
    const char* EntryPoint = "main";
    const char* Profile = nullptr; // 对于普通的Shader而言我们不希望你去指定Profile，因为在不同的平台上的Shader Model的支持程度是不同的，我们的Shader编译器会根据平台自动选择合适的Profile
    const char* FilePath = nullptr;
    bool EnableDebugInfo = false;
};
	
// 输入元素描述符
struct InputElementDesc
{
	const char* SemanticName;           		  // 语义名称
	uint32_t SemanticIndex;              		  // 语义索引
	RHITextureFormat Format;                      // 数据格式
	uint32_t InputSlot;              			  // 输入槽索引（0-15）
	uint32_t AlignedByteOffset;              	  // 字节偏移量（使用 APPEND_ALIGNED_ELEMENT 自动计算）
	InputClassification InputSlotClass;           // 输入分类（顶点数据或实例数据）
	uint32_t InstanceDataStepRate;                // 实例数据步进率（每几个实例取一次数据）
    
	// 辅助常量
	static constexpr uint32_t APPEND_ALIGNED_ELEMENT = 0xFFFFFFFF;
};
}
