#pragma once

#include "Common/RHI_API.h"
#include "Common/RHIResourceType.h"
#include <vector>

namespace RHI
{
    class Device; // forward declaration

    /*
    根签名（布局定义）
    ┌─────────────────────────────────────────┐
    │ 参数0: CBV (b0)                          │  ← 说："参数0是一个常量缓冲区"
    │ 参数1: DescriptorTable (t0-t4)           │  ← 说："参数1是5个纹理"
    │ 参数2: Constants (根常量)                 │  ← 说："参数2是4个32位常量"
    └─────────────────────────────────────────┘

    描述符堆（实际数据）
    ┌─────────────────────────────────────────┐
    │ 槽位0: 纹理0的描述符                       │
    │ 槽位1: 纹理1的描述符                       │
    │ 槽位2: 纹理2的描述符                       │
    └─────────────────────────────────────────┘

    渲染时的绑定：
    SetGraphicsRootSignature(根签名)  →  告诉GPU用这个布局
    SetGraphicsRootDescriptorTable(1, 描述符堆的GPU地址)  →  告诉GPU数据在哪

    并且在PSO创建之前可以设置RootSignature 与 Descriptor，
    在创建PSO之后RootSignature不允许在进行改变，但是Descriptor可以改变。
    */

    struct DescriptorRangeDesc
    {
        /*
        记住RootParameterDesc所有的这一切没有任何的实际的资源，他们所作的仅仅只是设置了资源布局
        ShaderRegister 是唯一一个可以确定描述符表中的元素所指向的资源在Shader中的寄存器位置，
        比如ShaderRegister  = 0 那么对应Shader的资源开始寄存器就是b0，ShaderRegister   = 1 则为b1.

        如果Range0 有三个CBV资源，那么则从b0（举例）开始直到b2为止，如果Range1也是CBV类型，
        那么我们需要更新ShaderRegister 为 3 否则将会导致同一个寄存器有两个不同的值，但是如果是不同资源则不需要设置，
        因为类型不同则寄存器空间不同，不冲突。

        OffsetInDescriptorsFromTableStart代表了资源描述符列表的索引偏移，默认来说我们使用叠加的方式，
        比如Range0 有三个CBV资源 那么Range1就应该在3开始设置自己的指向，
        如果我们把Range1设置为0那么Range1就会将Range0 设置的指向覆盖，这样可以将不同的寄存器指向同一资源。

        但是一般来说我们不推荐你去更改OffsetInDescriptorsFromTableStart，
        但是ShaderRegister 在多个元素时基本上是必需要设置的。
        **/
        DescriptorRangeType RangeType = DescriptorRangeType::CBV;
        uint32_t NumDescriptors = 1u;    // 描述符数量
        uint32_t ShaderRegister = 0u;// 着色器寄存器起始号
        uint32_t RegisterSpace = 0u; // 寄存器空间号(space0 , space1 , space2 , ...)类似命名空间
        uint32_t OffsetInDescriptorsFromTableStart = ~0u;     // Offset -> Append
    };

    struct RootParameterDesc
    {
        RootParameterType Type;                             // 根参数类型
        ShaderVisibility Visibility = ShaderVisibility::All;// 可见性

        union
        {
            // ===== 描述符表 =====
            struct
            {
                uint32_t NumDescriptorRanges;
                const DescriptorRangeDesc* pDescriptorRanges;
            } DescriptorTable;

            // ===== 根描述符（CBV / SRV / UAV）=====
            struct
            {
                uint32_t ShaderRegister;// 着色器寄存器起始号
                uint32_t RegisterSpace; // 寄存器空间号(space0 , space1 , space2 , ...)类似命名空间
            } Descriptor;

            // ===== 根常量 =====
            struct
            {
                uint32_t ShaderRegister;// 着色器寄存器起始号
                uint32_t RegisterSpace; // 寄存器空间号(space0 , space1 , space2 , ...)类似命名空间
                uint32_t Num32BitValues;
            } Constants;
        };
    };

    struct RootSignatureDesc
    {
        // 一个根签名可以有多个槽位，每个 RootParameterDesc 就是根签名里的一个“槽位”。
        std::vector<RootParameterDesc> Parameters;
        RootSignatureFlags Flags = RootSignatureFlags::None;
    };

    class RHI_API RHIRootSignature
    {
    public:
        virtual ~RHIRootSignature() = default;

        virtual bool Initialize(Device* device, const RootSignatureDesc& desc) = 0;
        virtual void Shutdown() = 0;

        virtual bool IsValid() const = 0;
    };
}
