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

    // TODO: 根签名参数
    struct RootParameterDesc
    {
        RootParameterType Type;  // 根参数类型
        uint32_t ShaderRegister;  // 着色器寄存器起始号
        uint32_t RegisterSpace;  // 寄存器空间号(space0 , space1 , space2 , ...)类似命名空间
        ShaderVisibility Visibility = ShaderVisibility::All; // 可见性
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
