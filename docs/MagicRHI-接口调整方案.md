# Magic-RHI 接口调整方案

> 面向对象：`Engine/Source/Runtime/RHI`（公共接口）+ `D3D12RHI` / `VulKanRHI`（两个实现）
> 依据：`docs/MagicRHI-问题清单.md` 48 条静态走查结论。复核基准：HEAD `88c62df`（提交于 2026-11-05 19:35，工作区干净），复核日 **2026-11-06**。`Binary\Debug` 下现有 `Magic-RHI.dll` / `Magic-Core.dll` / `Magic-D3D12RHI.dll` / `Magic-VulKanRHI.dll` 与三个测试 exe，时间戳均为 2026-11-05 **19:27:36~38**——晚于全部源文件最后修改时间（19:27:20）、早于 HEAD 提交 8 分钟，故这批产物与本次复核的源码是同一状态（Debug 全量链接成功）；本次复核**未重新运行**任何产物，所有落地状态与结论仍是代码路径推演。
> 结论先行：接口词汇（RootSignature / OMSetRenderTargets / Barrier）**不推翻**，只做 5 处增量 + 3 条契约澄清；Vulkan 侧最大的一件事（"一张表一个 descriptor set"）属于实现重构，**不进公共接口**。
> 本次复核更新（2026-11-06）：基线由 `18d30c2` 推进到 `88c62df`。其间只有 0c0436c / 88c62df 两个提交改动运行时代码——`GraphicsQueueSwapChainInfo` 及其 `_Vk` 字段已被 `GraphicsQueueRequirements`（多态基类）+ 两端 `Internal` 里的派生结构取代（详见 1.2，这直接推翻了本文原先引用它的那条论证）。**A~F 六项改动仍全部未落地**，全文行号已按新基线重校。c99617c 的 Vulkan 命令侧 `vkCmdBindPipeline` 与 `vkCmdSetViewport/Scissor` 仍在，但 **PSO 创建侧未跟上**（图形分支仍 `return false`，未声明任何 `VK_DYNAMIC_STATE_*`）；详见第 0 节状态列与第 5 节。

---

## 0. 一张表看清改什么

| # | 改动 | 性质 | 位置 | 掉的清单条目 | 落地状态（2026-11-06） |
|---|---|---|---|---|---|
| A | 视图创建带 `TextureViewDesc` | 加参数（有默认值） | `RHIInterface.h` / `RHIDefinitions.h` | P1-11、P1-13、P1-3（采样器那半） | **未落地**。公共头：`RHIInterface.h:132,144,150` 仍是旧签名（无 `TextureViewDesc` 尾参），`RHIDefinitions.h` 无 `RHITextureViewType`/`TextureViewDesc`。后端：`DescriptorViewVulKan.cpp:90-98,100-108,110-118,120-128` 与 `DescriptorViewD3D12.cpp:91-99,111-119,121-129` 仍只 `Allocate()`，未建 `VkImageView`/`VkSampler`/RTV/DSV |
| B | 描述符表按根签名布局分配 set | 纯 Vulkan 实现 | `DescriptorHeapVulKan.*`、`CommandListVK_RootSignature.cpp` | P1-1、P1-2、P2-9 | **未落地**。`DescriptorHeapVulKan.cpp:170-213` 仍 1-binding、`binding=0`、`descriptorCount=1`；`CommandListVK_RootSignature.cpp:93-95,197-199` 写死 `{Standard,0}` 并丢弃 `offsetInDescriptorsFromTableStart`；`Free`（`DescriptorHeapVulKan.cpp:85-117`）仍无 `vkFreeDescriptorSets` |
| C | 显式 `BeginRenderPass / EndRenderPass` | 新增纯虚函数 | `RHICommandList.h` | P0-2、P1-5、P1-16、P2-6 | **未落地**。`RHICommandList.h` 无 `BeginRenderPass/EndRenderPass` 声明；两端命令列表头（`RHICommandListVulKan.h:77-79`、`RHICommandListD3D12.h:62-64`）均无对应方法；Vulkan 仍由 `OMSetRenderTargets` 隐式开 dynamic rendering（`CommandListVK_OutputMerger.cpp:92`）。P0-2 的命令侧 `vkCmdBindPipeline` 已接（见 D 行），但 pass 生命周期接口本身未建 |
| D | 动态状态契约（viewport / scissor / topology / blend / depth） | 注释 + Vulkan PSO | `RHICommandList.h`、`RHIPipelineStateVulkan.cpp` | P0-3、P0-4、P1-4、P1-8 | **部分落地（仅 Vulkan 命令侧 viewport/scissor + PSO bind 命令）**。已通：`CommandListVK_PipelineState.cpp:31,36` 已 `vkCmdBindPipeline` 并记录 `m_pCurrentGraphicsPSO:32` / `m_pCurrentComputePSO:37`；`CommandListVK_Rasterizer.cpp:20,36` 已 `vkCmdSetViewport/Scissor`（Y 翻转见 :12-16）。未通：IA 三函数空（`CommandListVK_InputAssembly.cpp:10-12,14-16,18-20`）；blend/depth 两端空（VK `CommandListVK_OutputMerger.cpp:113-116,118-121`；D3D12 `CommandListDX12_OutputMerger.cpp:32-35,37-40`）；**PSO 创建侧未通**（`RHIPipelineStateVulkan.cpp:156-158` 图形分支仍 `return false`；整个 VulKanRHI 后端 grep 无 `vkCreateGraphicsPipelines` / `VK_DYNAMIC_STATE` / `VkDynamicState` / `pDynamicState` / `VkPipelineViewportStateCreateInfo`）；公共头无静态/动态状态分区注释 |
| E | `Map/Unmap` 带范围 + `Invalidate/Flush` | 改签名（默认参数） | `RHIResource.h` | P2-13、P2-14 | **未落地**。`RHIResource.h:354,358` 仍是无参 `Map()/Unmap()`，无 `InvalidateMappedRange/FlushMappedRange` 虚函数；D3D12 仍传 `(0,0)` 范围（`RHIResourceD3D12.h:67,85`）；Vulkan readback 选 `HOST_VISIBLE|HOST_CACHED`（`RHIResourceVulKan.cpp:20`），`Map:269`/`Unmap:287` 无 invalidate/flush |
| F | `RHI_ALL_SUBRESOURCES` 常量 | 常量 + 注释 | `RHIDefinitions.h` | P1-18 的一部分 | **未落地**。`RHIDefinitions.h` 无 `RHI_ALL_SUBRESOURCES`；`ResourceTransitionBarrier:43` 默认值仍 `UINT_INITIAL_VALUE(0)`，注释仍写 `-1 for all`；`TextureDesc:84-95` 无 `InitialState`；Vulkan 仍按 `UINT32_MAX` 判全部（`CommandListVK_ResourceBarrier.cpp:165`），swapchain 首用特判仍在（`:141,155-163`） |
| — | 顶点偏移 + stride 契约 | 注释（不改签名） | `RHICommandList.h` | P1-15 | **未落地**。`CommandListDX12_InputAssembly.cpp:88-104` 仍忽略 `pOffsets`（`BufferLocation:97` 用基址，`SizeInBytes:99` 用整缓冲） |
| — | 不改 | 明确不做 | — | 见第 11 节 | 维持不做（复核未发现与现状矛盾） |

预计触碰：公共头 4 个、D3D12 实现 5 个、Vulkan 实现 7 个、测试 0~1 个（取决于是否走兼容路径）。

---

## 1. 判定原则：什么该进接口，什么不该

三条硬标准，缺一不进：

1. **两个后端都需要**。只有 Vulkan 要的信息（set / binding / layout 具体值）一律留在 `VulKanRHI/Internal`，不许出现在 `RHI/Public`。
2. **无法从现有参数推出**。能从 `RootSignatureDesc`、`BufferDesc`、`GraphicsPipelineStateDesc` 算出来的，不新增字段。
3. **每项改动必须能销掉清单里至少一条**。否则是抽象洁癖。

### 1.1 先纠正一个容易重复造的轮子

我上一轮口头提到的"接口里没有 state→layout 映射"**不准确**。映射已经写好了，在 `CommandListVK_ResourceBarrier.cpp:9-33`：

- `ConvertToVkImageLayout`（`:9-33`）把 `RHIResourceState` 逐位翻成 `VkImageLayout`；
- `ConvertToVkAccessFlags`（`:36-58`）、`ConvertToVkPipelineStageFlags`（`:61-92`）同理。

所以**不要再引入一套平行的 `RHIResourceAccess` 枚举**——那等于把同一件事写两遍，两边还会漂。真正缺的是三件更小的事：首次布局从哪来（见第 3 节 F）、aspect 怎么定（`:95-103` 现在"只要沾深度就返回 DEPTH|STENCIL"）、子资源默认值语义矛盾（见 F）。

### 1.2 后端口径不进公共头：88c62df 之后有了正式出口（原"先例"论证已失效）

上一版本文曾以旧基线下 `RHIDefinitions.h:29-42` 的 `GraphicsQueueSwapChainInfo` 作为"允许 `_Vk` 字段进公共头"的先例。**这条依据已经不成立**——0c0436c / 88c62df 把这个结构拆掉了，改成了"公共头留中性基类、后端专有字段下沉到各自 Internal"：

这个结构在公共头只剩一个跨 API 中性的多态基类（`RHIDefinitions.h:29-35`）：

```cpp
struct GraphicsQueueRequirements
{
    // Since each backend requires different parameters, we leave it to each backend to implement.
    virtual RHIType GetType() const = 0;

    GraphicsQueueRequirements(){}
};
```

后端专有字段分别落在两端 `Internal` 头的派生结构里：

- `GraphicsQueueRequirementsVulKan`（`VulKanRHI/Internal/RHICommandListVulKan.h:158-170`）：`VkSwapchainKHR` / `VkSemaphore` / `imageIndex` / `SwapChainBackBufferCount` / `VSync`；
- `GraphicsQueueRequirementsD3D12`（`D3D12RHI/Internal/RHICommandListD3D12.h:118-128`）：`IDXGISwapChain1*` / `imageIndex` / `VSync`。

交接方式也从"按值传一个统一结构"改成"传基类指针 + 运行时类型校验"：`SetGraphicsQueueRequirements(GraphicsQueueRequirements*)`（`RHICommandList.h:403`）里先比 `GetType()`，再 `SafeCast` 到具体派生结构（`RHICommandListD3D12.cpp:289-304`、`RHICommandListVulKan.cpp:289-304`）；反方向由 `std::unique_ptr<GraphicsQueueRequirements> GetGraphicsQueueRequirements() const`（`RHIInterface.h:329`）返回。

这条新取向比旧先例更强，红线也更清楚：

1. 公共头现在**一个 `_Vk` 字段都没有**，`RHI/Public` 下也检索不到 `Vk` / `D3D12_` 类型的句柄或布局（本次全文检索 0 命中）；
2. 后端专有信息的合法去处是"后端 `Internal` 头的派生结构"，而不是公共头里的可选字段；
3. 所以第 2 节的 `TextureViewDesc` 继续保持跨 API 中性（不出现 `Vk` 字样），第 3 节的 `InitialState` 复用已有的 `RHIResourceState`——**不要**借新基类往回加 `_Vk` 式字段，那等于把刚拆掉的东西原地装回去。

> 这个新模式本身还有一处待补：基类没有虚析构，而 `GetGraphicsQueueRequirements()` 用 `std::unique_ptr<GraphicsQueueRequirements>` 返回派生对象（`RHIInterface.h:329`、`SwapChainVulkan.cpp:214-223`、`SwapChainD3D12.cpp:192-199`），经基类指针析构属 UB。已记为问题清单 **N6**；沿用这个模式前先补 `virtual ~GraphicsQueueRequirements() = default;`。

---

## 2. 改动 A：视图创建必须带视图描述

> **落地状态（2026-11-06）：未落地。** 公共头签名未变（`RHIInterface.h:132,144,150`），两端后端纹理/采样器/RTV/DSV 入口仍只分配槽位（证据见 2.1）。

### 2.1 现状（三条证据）

公共接口给了四个建视图的函数，但**没有给任何"视图长什么样"的信息**：

```
RHIInterface.h:132   virtual RHIDescriptorHandle CreateStandardHeapDescriptorView(RHITexture* Texture,DescriptorRangeType Type) = 0;
RHIInterface.h:144   virtual RHIDescriptorHandle CreateRTVHeapDescriptorView(RHITexture* Texture) = 0;
RHIInterface.h:150   virtual RHIDescriptorHandle CreateDSVHeapDescriptorView(RHITexture* Texture) = 0;
```

实现侧因此全都退化成"只分配一个槽位"：

| 位置 | 现在做的事 | 后果 |
|---|---|---|
| `DescriptorViewVulKan.cpp:90-98` | 检查堆 → `Allocate()` → 直接 return | 没建 `VkImageView`，纹理 SRV 槽位是空的（P1-11） |
| `DescriptorViewVulKan.cpp:100-108` | 同上（采样器） | 没建 `VkSampler`（P1-3 的一半） |
| `DescriptorViewVulKan.cpp:110-118`、`:120-128` | 同上（RTV / DSV） | 附件视图不存在 |
| `DescriptorViewD3D12.cpp:91-99` | 用 **sampler 堆** 分配，`Type` 参数完全没用 | 纹理描述符进错堆（P1-13） |
| `DescriptorViewD3D12.cpp:111-119`、`:121-129` | 只 `Allocate()`，没有 `CreateRenderTargetView` / `CreateDepthStencilView` | RTV/DSV 句柄指向未初始化的描述符 |

对比同文件的 buffer 分支：`DescriptorViewD3D12.cpp:32-80` 认真填了 `D3D12_CONSTANT_BUFFER_VIEW_DESC` / `SHADER_RESOURCE_VIEW_DESC` / `UNORDERED_ACCESS_VIEW_DESC`。**说明不是实现不想做，是接口没给够信息**——纹理缺格式、缺维度、缺 mip/layer 范围，实现无从填 DESC。

### 2.2 改法

在 `Common/RHIDefinitions.h` 紧跟 `TextureDesc` 之后加：

```cpp
// 视图类型：只描述"看得见哪一面"，不描述存储
enum class RHITextureViewType : uint8_t
{
    Texture1D = 0,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    TextureCube,
    Texture3D
};

struct TextureViewDesc
{
    RHITextureViewType ViewType   = RHITextureViewType::Texture2D;
    RHITextureFormat   Format     = RHITextureFormat::Unknown; // Unknown = 沿用资源自身格式
    uint32_t           FirstMip   = 0;
    uint32_t           NumMips    = RHI_ALL_SUBRESOURCES;      // 全部
    uint32_t           FirstLayer = 0;
    uint32_t           NumLayers  = RHI_ALL_SUBRESOURCES;
};
```

`RHIInterface.h` 三个签名各加一个**带默认值**的参数（默认参数 = 现有调用方零改动，`:132` / `:144` / `:150`）：

```cpp
virtual RHIDescriptorHandle CreateStandardHeapDescriptorView(
    RHITexture* Texture, DescriptorRangeType Type, const TextureViewDesc& View = TextureViewDesc{}) = 0;

virtual RHIDescriptorHandle CreateRTVHeapDescriptorView(
    RHITexture* Texture, const TextureViewDesc& View = TextureViewDesc{}) = 0;

virtual RHIDescriptorHandle CreateDSVHeapDescriptorView(
    RHITexture* Texture, const TextureViewDesc& View = TextureViewDesc{}) = 0;
```

> 不加新函数名，只加尾参默认值。理由：一旦新增重载，`nullptr` 实参会撞上重载决议，`Tests/` 与 `SwapChain` 里已有的写法得逐个改。

### 2.3 两侧落地

**Vulkan**（`DescriptorViewVulKan.cpp:90-128` 四个函数）

1. `SafeCast<TextureVulKan>(Texture)` 拿 `GetImage()`、`GetFormat()`；
2. 由 `ViewType` 推 `VkImageViewType`，`Format = View.Format == Unknown ? GetFormat() : View.Format`；
3. `aspect`：`IsDepthFormat(format)` 决定 `DEPTH`，`IsStencilFormat(format)` 决定补 `STENCIL`（两个函数已在 `RHITypes.h:405-434`，别再自己写 switch）；
4. `vkCreateImageView` → `DescriptorHeapVulKan::SetSRVDescriptor(...)`（该函数已存在，`DescriptorHeapVulKan.h:60`）；
5. RTV / DSV 走 `SetRTVDescriptor` / `SetDSVDescriptor`（`DescriptorHeapVulKan.h:62-63`）。

**D3D12**（`DescriptorViewD3D12.cpp:91-129`）

1. `:93` 的 `m_pSamplerHeap` 换成 `m_pStandardHeap`；
2. 按 `Type` 分支填 DESC：SRV 用 `D3D12_SRV_DIMENSION_TEXTURE2D`、`MostDetailedMip = View.FirstMip`、`MipLevels = NumMips`；UAV 同理；
3. RTV：`D3D12_RENDER_TARGET_VIEW_DESC{ Format, ViewDimension = TEXTURE2D, Texture2D.MipSlice = FirstMip }` → `CreateRenderTargetView`；
4. DSV：`D3D12_DEPTH_STENCIL_VIEW_DESC` + `Texture2D.MipSlice` → `CreateDepthStencilView`；
5. `SetSRVDescriptor` / `SetRTVDescriptor` 等已就绪（`DescriptorHeapD3D12.h:58-63`）。

### 2.4 验证（不用等整个测试跑通）

```
1) 造一张 4×4 R8G8B8A8 纹理 → CreateStandardHeapDescriptorView(Tex, SRV)
2) GetDescriptorHeapView(handle) 非空，且能 SafeCast 成 ShaderResourceViewVulKan / ShaderResourceViewD3D12
3) Vulkan: 该 SRV 的 VkImageView != VK_NULL_HANDLE；aspect == COLOR
4) 把 Format 改成 D32_FLOAT 再来一遍：aspect 必须变成 DEPTH（无 STENCIL）
```

第 4 步是专门防 P1-18 那种"沾深度就带 stencil"的写法回流。

---

## 3. 改动 F：子资源常量与首次布局

> **落地状态（2026-11-06）：未落地。** 常量 `RHI_ALL_SUBRESOURCES` 与 `TextureDesc::InitialState` 均未加入公共头；barrier 默认值/注释/实现三处口径矛盾依旧（证据见 3.1）。

### 3.1 现状

```cpp
RHIDefinitions.h:40-46
struct ResourceTransitionBarrier
{
    class RHIResource* pResource   = nullptr;
    uint32_t           Subresource = UINT_INITIAL_VALUE;          // 注释写 "Subresource Index (-1 for all)"
    RHIResourceState   StateBefore = RHIResourceState::Common;
    RHIResourceState   StateAfter  = RHIResourceState::Common;
};
```

`UINT_INITIAL_VALUE` 在 `RHIDefinitions.h:10` 定义为 `0`，而注释说 `-1` 表示全部；Vulkan 侧按 `UINT32_MAX` 判全部（`CommandListVK_ResourceBarrier.cpp:165`）。**注释、默认值、实现三处口径不一致**，调用方随手写默认值就等于只转 mip 0。

另一处同类问题：`CommandListVK_ResourceBarrier.cpp:141` 与 `:155-163` 用 `pTexture->IsFirstUseOfSwapChainImage()` 把 `oldLayout` 强改成 `UNDEFINED`。这说明"资源第一次被使用时的布局"这个信息接口没给，只能靠 swapchain 专用标志打补丁——**非交换链的纹理（比如离屏 RT）根本没有这个入口**，第一次 barrier 就会拿错 `oldLayout`。

### 3.2 改法

```cpp
// RHIDefinitions.h 顶部
static constexpr uint32_t RHI_ALL_SUBRESOURCES = 0xFFFFFFFFu;

// 修正默认值，让"默认 = 全部子资源"与注释一致
struct ResourceTransitionBarrier
{
    ...
    uint32_t Subresource = RHI_ALL_SUBRESOURCES;
    ...
};

// TextureDesc 末尾加一行（RHIResourceState 早已存在，见 RHITypes.h:468 起）
struct TextureDesc
{
    ...
    RHIResourceState InitialState = RHIResourceState::Common; // 创建时的布局来源
};
```

### 3.3 落地

- **Vulkan**：把 `:141` 的 `pTexture->IsFirstUseOfSwapChainImage()` 换成资源自身状态。具体：`TextureVulKan` 增 `bool m_bBarrierIssued = false`；`ResourceBarrier` 里

  ```cpp
  const bool bFirstUse = !pTexture->HasBarrierIssued();
  VkImageLayout oldLayout = bFirstUse ? VK_IMAGE_LAYOUT_UNDEFINED
                                      : ConvertToVkImageLayout(trans.StateBefore);
  ```

  首次下发后置位，于是 `CommandListVK_ResourceBarrier.cpp:155-163` 整段 swapchain 特判连同 `IsFirstUseOfSwapChainImage()` 一起删掉。`TextureDesc::InitialState` 用来取代"创建时状态"的默认值——离屏 RT 也能走同一条路。
- **D3D12**：`InitialState` 直接映射 `D3D12_RESOURCE_STATES`，创建资源时用；`Subresource = -1` 时填 `D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES`。

### 3.4 验证

- 离屏 RT（非交换链）第一次 barrier（`Common → RenderTarget`）后，`vkGetImageMemoryRequirements` 之类的校验层（`VK_LAYER_KHRONOS_validation`）不得报 `oldLayout` 不匹配。
- 同一纹理第二次 barrier（`RenderTarget → Common`）不许再写 `UNDEFINED`。

---

## 4. 改动 C：显式的 render pass 阶段（这条最关键）

> **落地状态（2026-11-06）：未落地。** 公共头无 `BeginRenderPass/EndRenderPass`，两端命令列表均无对应方法；Vulkan 仍由 `OMSetRenderTargets` 隐式开 dynamic rendering。已发生的变化：`SetPipelineState` 命令侧已接 `vkCmdBindPipeline`（c99617c），不再是 TODO，但图形 PSO 本身创建失败（见第 5 节 D），且"pass 内"状态接口仍未定义。

### 4.1 现状

`OMSetRenderTargets` 在两端语义完全不同，接口没有表达这个差异：

| | D3D12 | Vulkan |
|---|---|---|
| 实现 | `CommandListDX12_OutputMerger.cpp:29` 直接 `OMSetRenderTargets` | `CommandListVK_OutputMerger.cpp:92` 在函数结尾**顺手 `BeginDynamicRendering`** |
| 附件清屏 | 独立 `ClearRenderTargetView`（`CommandListDX12_Clear.cpp:11`） | 只能 `vkCmdClearAttachments`，且要求 pass 已开（`CommandListVK_Clear.cpp:9`） |
| 深度清除 | `CommandListDX12_Clear.cpp:14-17` 空 | `CommandListVK_Clear.cpp:36-38` 空 |
| barrier | 随时可发 | 被迫先 `EndDynamicRendering`（`CommandListVK_ResourceBarrier.cpp:111-113`） |

由此连锁出三条 P0/P1：

- `CommandListVK_PipelineState.cpp:5-39` 的 `SetPipelineState` 已做类型/有效性校验并在 `:31/:36` 调 `vkCmdBindPipeline`、记录 `m_pCurrentGraphicsPSO:32` / `m_pCurrentComputePSO:37`；但图形 PSO 在创建侧恒无效（`RHIPipelineStateVulkan.cpp:156-158`），会在 `:23-27` 的 `IsValid()` 校验处早退，且"pass 内"这个状态接口仍未定义；
- `CommandListVK_OutputMerger.cpp:113-116,118-121` 的 blend / depth 只能空——extended dynamic state 要求在 pass 内下发；
- `CommandListVK_OutputMerger.cpp:45` 的 `m_CurrentRenderTargets.push_back` 只进不出，`EndDynamicRendering`（`:104-111`）不清 `m_CurrentRenderTargets` / `m_CurrentRenderArea`，导致第二个 pass 里 `ClearRenderTargetView` 的索引查找（`CommandListVK_Clear.cpp:15-19`）可能命中上一帧残留。

### 4.2 改法

`RHICommandList.h` 在 `OMSetRenderTargets`（`:123`）之前插入：

```cpp
enum class RHILoadOp : uint8_t { Load = 0, Clear, DontCare };

struct RenderPassDesc
{
    RHIRenderTargetView* pColorViews[DRAW_MAX_RENDER_TARGETS] = {};
    uint32_t             NumColorTargets = 0;
    RHIDepthStencilView* pDepthView      = nullptr;

    RHILoadOp ColorLoadOp = RHILoadOp::Load;
    RHILoadOp DepthLoadOp = RHILoadOp::Load;
    RHILoadOp StencilLoadOp = RHILoadOp::DontCare;

    float   ColorClear[DRAW_MAX_RENDER_TARGETS][4] = {};
    float   DepthClear   = 1.0f;
    uint8_t StencilClear = 0;

    bool bPreserveColor   = true;   // false → storeOp = DONT_CARE（省带宽）
    bool bPreserveDepth   = true;
};

// 显式开始 / 结束一个 pass。D3D12 侧只做 OM 绑定 + 清屏；Vulkan 侧构造 VkRenderingInfo。
virtual void BeginRenderPass(const RenderPassDesc& Desc) = 0;
virtual void EndRenderPass() = 0;
```

同时改写两个方法的**契约注释**（不动签名）：

| 方法 | 新契约 |
|---|---|
| `OMSetRenderTargets`（`:123`） | 只登记"下一次 pass 用哪些附件"，不立即开始 pass |
| `ClearRenderTargetView`（`:183`） | 只在 pass 内有效；是否真的走 `vkCmdClearAttachments` 由后端决定 |
| `ClearDepthStencilView`（`:191`） | 同上，且**必须**实现（现在两端都空） |

### 4.3 迁移策略：先兼容，再翻转

当前唯一的帧循环调用方是 `Tests/HelloTriangleTest/HelloTriangleTest.cpp:277-336`，顺序为
`ResourceBarrier(:295) → OMSetRenderTargets(:298) → ClearRenderTargetView(:301) → SetGraphicsRootSignature(:303) → SetPipelineState(:304) → IASetPrimitiveTopology(:306) → IASetVertexBuffers(:309) → RSSetViewports(:315) → RSSetScissorRects(:318) → Draw(:320)`。

- **里程碑 1（只加接口）**：Vulkan 侧 `BeginRenderPass` 实现为"真正开 pass"；`OMSetRenderTargets` 保留旧行为（`:11-15` + `:92` 隐式开 pass）。测试文件**一行不改**，先让它编过。
- **里程碑 2（翻转默认）**：`OMSetRenderTargets` 只登记；测试 `:298-301` 改成：

```cpp
RHI::RenderPassDesc passDesc{};
passDesc.NumColorTargets = 1;
passDesc.pColorViews[0]  = pRTV;
passDesc.ColorLoadOp     = RHI::RHILoadOp::Clear;
passDesc.ColorClear[0][0] = 0.0f; /* G/B/A 同 */
cmdLists[ImageIndex]->BeginRenderPass(passDesc);
... // Draw 之后
cmdLists[ImageIndex]->EndRenderPass();
```

`ClearRenderTargetView(:301)` 可删（改由 `ColorLoadOp::Clear` 承担），也可保留（D3D12 侧两者等价）。

### 4.4 两侧落地

**Vulkan**（`CommandListVK_OutputMerger.cpp` 为主）

1. `BeginRenderPass`：按 `Desc` 组装 `VkRenderingInfoKHR`，`loadOp` 取 `ColorLoadOp` / `DepthLoadOp`，`imageLayout` 从资源当前状态推。注意 `ConvertToVkImageLayout` / `ConvertToVkAccessFlags` / `ConvertToVkPipelineStageFlags` 现在是 `CommandListVK_ResourceBarrier.cpp:9-92` 里的匿名 namespace 函数，OutputMerger 用不到——需要**新建**一个头把这三个提取出来共用：例如 `VulKanRHI\Internal\VkConvert.h`。（核实：`RHITypesVulkan.h` 与 `VkConvert.h` 当前在仓库里都不存在；`Internal` 下现有 5 个头 `DescriptorHeapVulKan.h` / `RHICommandListVulKan.h` / `RHIPipelineStateVulkan.h` / `RHIResourceVulKan.h` / `RHIRootSignatureVulKan.h` 均不含这三个转换；跨后端公共类型头 `RHI/Public/Common/RHITypes.h` 也不含 VK 专属转换。）头建好后由 ResourceBarrier 与 OutputMerger 两个 cpp 共用，别在两处各写一份；
2. `EndRenderPass`：`vkCmdEndRenderingKHR` + **清 `m_CurrentRenderTargets` / `m_CurrentRenderArea`**（现在不清，就是 P2-6）；
3. `SetPipelineState`（`CommandListVK_PipelineState.cpp:5-39`）：`vkCmdBindPipeline` 已在 `:31/:36` 发出、当前 PSO 已记录在 `:32/:37`；待补的是"当前 pass 的附件格式"校验——与 PSO 的 `RenderTargetFormats`（`RHIPipelineState.h:45-46`）对不上就 `ThrowErrorMessage`；
4. `ClearDepthStencilView`：`vkCmdClearAttachments` + `aspectMask = DEPTH`（有 stencil 才加 `STENCIL`，**别照抄** `CommandListVK_ResourceBarrier.cpp:95-103` 的 `GetAspectMask`，它只要沾深度就返回 `DEPTH|STENCIL`，正是 P1-18）；
5. `RSSetViewports` / `RSSetScissorRects` 已在 `CommandListVK_Rasterizer.cpp:5-37` 落地（`vkCmdSetViewport:20`、`vkCmdSetScissor:36`，Y 翻转见 `:12-16`）；仍缺 `IASetPrimitiveTopology` / `IASetVertexBuffers` / `IASetIndexBuffer`（`CommandListVK_InputAssembly.cpp:10-12,14-16,18-20` 三函数空体），前提见第 5 节 D。

**D3D12**（`CommandListDX12_OutputMerger.cpp`）

1. `BeginRenderPass`：`OMSetRenderTargets` + 按 `LoadOp == Clear` 逐个 `ClearRenderTargetView`；depth 同理补上 `ClearDepthStencilView`；
2. `EndRenderPass`：空实现（D3D12 无对应概念）；
3. `ClearDepthStencilView`（`CommandListDX12_Clear.cpp:14-17`）：`ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | _STENCIL, depth, stencil, 0, nullptr)`。

### 4.5 验证

- Vulkan 校验层不报 `VUID-vkCmdDraw-None-*` 之外的 pass 顺序错误；
- 连续两帧 `Begin/Clear/Draw/End`，第二帧的清除框必须落在**本帧**的 `m_CurrentRenderArea`（P2-6 的回归点）；
- `SetPipelineState` 传入格式不匹配的 PSO（把 `RenderTargetFormats[0]` 改成 `R8G8B8A8_UNORM` 与 8 位后备缓冲一致/不一致各跑一次）→ 必须报错而不是画黑。

---

## 5. 改动 D：动态状态契约（不改签名，但要写死）

> **落地状态（2026-11-06）：部分落地。** 仅 Vulkan 命令侧的 viewport / scissor 与 PSO bind 命令已通；IA 拓扑/顶点/索引、两端 blend / depth 仍空；**PSO 创建侧未通，且未声明任何 `VK_DYNAMIC_STATE_*`**；公共头也还没有静态/动态状态分区注释。命令侧先于 PSO 侧落地，当前组合不能被表述为"整条动态状态链路已落地"。

### 5.1 为什么必须约定

`RSSetViewports` / `RSSetScissorRects` / `IASetPrimitiveTopology` / `OMSetBlendState` / `OMSetDepthStencilState` 这五个方法，在 D3D12 侧：视口/裁剪/拓扑已实现（`CommandListDX12_Rasterizer.cpp:5-37`、`CommandListDX12_InputAssembly.cpp:76-86`），但 `OMSetBlendState` / `OMSetDepthStencilState` 仍是 TODO 空体（`CommandListDX12_OutputMerger.cpp:32-35,37-40`）。在 Vulkan 侧：viewport / scissor 已实现（`CommandListVK_Rasterizer.cpp:5-37`），IA 三函数仍空（`CommandListVK_InputAssembly.cpp:10-12,14-16,18-20`），blend / depth 仍空（`CommandListVK_OutputMerger.cpp:113-116,118-121`）。Vulkan 只有在 PSO 创建时声明了 `VK_DYNAMIC_STATE_*` 才能动态改；接口里目前没有任何地方表达这件事，公共头也没有契约注释。

### 5.2 约定内容（写进 `RHICommandList.h:78-136` 这段 IA / Rasterizer / OutputMerger 声明区的正上方）

> 本 RHI 的所有管线状态分两类：
> **静态（进 PSO）**：顶点输入布局、shader、render target 格式、depth 格式、rasterizer 的 fill mode / cull mode。
> **动态（每条命令可改）**：viewport、scissor、primitive topology、blend、depth-stencil、stencil ref。
> Vulkan 后端创建 graphics PSO 时**必须**声明对应的 `VK_DYNAMIC_STATE_*`；blend / depth 走 `VK_EXT_extended_dynamic_state`（无扩展时允许在 `BeginRenderPass` 内重建 pipeline，但必须在 PSO 缓存里命中）。

### 5.3 Vulkan 落地要点

`RHIPipelineStateVulkan.cpp:156-158` 现在是 `Type = Graphics; return false;`——连 `vkCreateGraphicsPipelines` 都没调（对照 `:216-239` 的 compute 分支是真建的，`vkCreateComputePipelines:224-234`）。

**VK_DYNAMIC_STATE 核实结论（2026-11-06 复检）**：在整个 `VulKanRHI` 目录 grep `vkCreateGraphicsPipelines` / `VK_DYNAMIC_STATE` / `VkDynamicState` / `pDynamicState` / `dynamicState` / `VkPipelineViewportStateCreateInfo` / `viewportCount` / `scissorCount`，**0 命中**（`VkViewport` 仅出现在命令侧 `CommandListVK_Rasterizer.cpp:9` 的视口数组里，与 PSO 创建无关）。

现状属于哪一种错，需要和后面两种典型错误区分开：

- **当前**：图形分支在 `:156-158` 直接 `return false`，根本没走到创建调用；因此既不是"viewportCount=0"，也不是"viewportCount=1 但漏声明动态态"——`VkPipelineViewportStateCreateInfo` 与 `VkPipelineDynamicStateCreateInfo` 两个 struct 都未构造，计数与动态态声明**整体缺失**。图形 PSO 恒无效（`IsValid()` 为 false，`CommandListVK_PipelineState.cpp:23-27` 早退），命令侧 `vkCmdSetViewport/Scissor` 虽已发出，但动态状态尚未真正作用于管线。
- **后续补创建时**，两种错误要分开防：① 只补 viewport state 而漏 dynamic state（`viewportCount=1` 但不声明 `VIEWPORT/SCISSOR/PRIMITIVE_TOPOLOGY`）→ 命令侧 `vkCmdSetViewport/Scissor` 触发校验层 VUID，视口/裁剪不生效；② 漏 `VkPipelineViewportStateCreateInfo` 或 `viewportCount/scissorCount=0` → PSO 创建阶段即校验失败。

补这条时：

1. `VkPipelineDynamicStateCreateInfo` 至少填 `VIEWPORT / SCISSOR / PRIMITIVE_TOPOLOGY`；并配 `VkPipelineViewportStateCreateInfo` 填 `viewportCount = 1` / `scissorCount = 1`（或从 `desc` 取）；两者必须同时存在；
2. `VkPipelineRenderingCreateInfoKHR` 的 `colorAttachmentFormats` 从 `desc.RenderTargetFormats`（`RHIPipelineState.h:45`）取，`depthAttachmentFormat` 从 `DepthStencilFormat`（`:46`）取；
3. `VkVertexInputBindingDescription.stride` 取 `RHIBuffer::GetStride()`（`RHIResource.h:327`），**不要从 `InputElementDesc` 自己算**；
4. `pipelineInfo.layout = rootSig->GetPipelineLayout()`（`:219` 已有此写法）。

### 5.4 验证

- 同一个 PSO，改 `RSSetViewports` 后三角形必须跟着缩放（证明 dynamic viewport 生效）；
- 不重建 PSO 的前提下改 `IASetPrimitiveTopology(TriangleStrip)`，画面不能黑。

---

## 6. 改动 B：描述符表按根签名布局分配 set（纯实现，不进接口）

> **落地状态（2026-11-06）：未落地。** 表级 set 能力未加；`AllocateDescriptorSet` 仍是 1-binding / binding 0；绑定点写死 `{Standard,0}`、丢弃 offset；root CBV/SRV/UAV 六个函数仍 TODO（`CommandListVK_RootSignature.cpp:107-114,116-123,125-132,211-218,220-227,229-236`）。push constant 路径已落地（`vkCmdPushConstants:165,269`），不在本节范围内。

### 6.1 现状

Vulkan 的 descriptor 分配策略是"**一个资源一个 set、binding 固定 0**"：

- `DescriptorHeapVulKan.cpp:170-213`（`AllocateDescriptorSet`）：`binding.binding = 0`、`descriptorCount = 1`，注释（`:172-176`）明说"每个资源一个 set，方便后面在 CommandList 里灵活绑定"；
- `CommandListVK_RootSignature.cpp:93-104` 于是只能：

```cpp
DescriptorSetBinding binding;
if (!pHeap->GetDescriptorSetBinding({RHIDescriptorHeapType::Standard,0},binding)) return;  // ← 写死 0 号
vkCmdBindDescriptorSets(..., firstSet, 1, &binding.set, 0, nullptr);                      // ← 只绑 1 张
```

这就是 P1-2 的原文现场（`layoutInfo`、`tableParam` 取出来一次都没用，`offsetInDescriptorsFromTableStart` 直接被丢弃）。它造成的直接后果：**只要一个 PSO 里有两张 descriptor table，或者表起点不是 0（多个 SRV 连续放在同一个堆里就是这种情况），两次绑定都会落到同一份 set**——采样到错误资源，而且不报错，比 P1-1 更难发现。

根签名侧的信息其实是齐的：`RHIRootSignature.h:38-64` 的 `DescriptorRangeDesc` 有 `RangeType（:59）/ NumDescriptors（:60）/ ShaderRegister（:61）/ RegisterSpace（:62）/ OffsetInDescriptorsFromTableStart（:63）`；`RHIRootSignatureVulKan.h:31` 还存了 `DescriptorSetIndex`。

那段注释还写死了两条约束，正好是本节要用的（`RHIRootSignature.h:42-50`）：**"RootParameterDesc 这一切没有任何的实际资源，他们所作的仅仅只是设置了资源布局"**、**"描述符表中的所有 Range 都必须来自同一个堆中"**。前者说明公共接口本来就只该描述布局、不该塞寻址细节；后者说明 Vulkan 侧可以把"一张表 = 一个 heap 的连续槽位"当作硬前提。

### 6.2 改法（Vulkan 内部）

**结论：公共接口一个字都不用改。** `SetGraphicsRootDescriptorTable(rootParam, heap, offset)` 三个参数已经足够 Vulkan 自己算出"这张表 = set 第 X 号、binding 从 `ShaderRegister` 起、共 `NumDescriptors` 个、从 heap 第 `offset` 槽开始"。

`DescriptorHeapVulKan` 增一个**表级 set** 能力：

```cpp
// 按"根签名表布局"取/建一张 set，并用 heap 里从 firstIndex 起的连续槽位填充
bool GetTableSet(VkDescriptorSetLayout TableLayout,      // 由根签名 DescriptorRangeDesc 建
                 uint32_t FirstIndex,                    // offsetInDescriptorsFromTableStart
                 uint32_t RangeCount,                     // 该 layout 的 binding 数量
                 VkDescriptorSet& OutSet);
```

实现要点：

0. **先解 offset 哨兵**。`OffsetInDescriptorsFromTableStart` 的默认值是 `~0u`（`RHIRootSignature.h:63`，注释"Offset -> Append"），含义是"接在上一个 range 后面"。算出真实起点要沿 `DescriptorTable.NumDescriptorRanges`（`RHIRootSignature.h:79`）顺序累加，**不能把 `0xFFFFFFFF` 直接当索引用**——否则就是 `m_Descriptors` 越界读。
1. 首次 miss 时 `vkAllocateDescriptorSets`（layout 传 `RHIRootSignatureVulKan::GetDescriptorSetLayout(i).Layout`，`RHIRootSignatureVulKan.h:60`）；
2. 逐 binding 组 `VkWriteDescriptorSet`：`dstBinding = Range.ShaderRegister`（`RHIRootSignature.h:61`），`pBufferInfo` / `pImageInfo` 从 `m_Descriptors[firstIndex + k]` 里取（`DescriptorHeapVulKan.h:12-17` 已有 `pCBV/pSRV/pUAV/pSampler`）；
3. **缓存 key = (TableLayout, FirstIndex, RangeCount)**，同一帧内重复绑定不重复建 set；
4. `Free`（`DescriptorHeapVulKan.cpp:85-117`）里同步失效该 index 参与的表 set——这一条同时把 P2-9（set 从不 `vkFreeDescriptorSets`）收口。

`CommandListVK_RootSignature.cpp:92-104` 换成（注意 `:92` 那行还挂着 `// TODO : Implement this method`，可下面 12 行其实已经在绑一次了——这种"TODO 与实现并存"是这轮最容易骗过审查的形态，改的时候顺手把注释删掉）：

```cpp
const uint32_t firstSet = slot.DescriptorSetIndex;
VkDescriptorSet tableSet = VK_NULL_HANDLE;
if (!pHeap->GetTableSet(layoutInfo.Layout, offsetInDescriptorsFromTableStart,
                        tableParam.DescriptorTable.NumDescriptorRanges, tableSet)) return;
vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_pCurrentGraphicsRS->GetPipelineLayout(), firstSet, 1, &tableSet, 0, nullptr);
```

> `tableParam.DescriptorTable.pDescriptorRanges`（`RHIRootSignature.h:80`）已经保留在 `m_Desc` 里（`RHIRootSignatureVulKan.h:82`），取参数用 `GetRootParameterDesc`（`:66`），够用。

### 6.3 根级 CBV/SRV/UAV（P1-1）单独处理

`CommandListVK_RootSignature.cpp:30-36` 的注释已经把三条路列好了；给出取舍：

| 方案 | 条件 | 取舍 |
|---|---|---|
| **推荐**：每个根描述符一张 1-binding 的 set，在 `SetGraphicsRootConstantBufferView` 里 `vkCmdBindDescriptorSets` 到 `slot` 对应号 | 无扩展要求 | 最稳；每帧每根描述符一次分配，用 6.2 的缓存池兜住 |
| `VK_EXT_push_descriptor` | 需 `vkCmdPushDescriptorSetKHR` | 少一次分配，但扩展检测要进 `DeviceVulKan::Initialize` |
| 改写已有 set 的 slot | 需要 `vkUpdateDescriptorSets` + 同步 | 会撞到"同一 set 被多条命令并发使用"的隐患，不推荐 |

`RHIRootSignatureVulKan.h:41-44` 的 `RootDescType / RootDescShaderRegister / RootDescRegisterSpace` 已经存好了映射，够推 binding。

### 6.4 验证

- 一张表里放 3 个 SRV（`NumDescriptors = 3`），`offsetInDescriptorsFromTableStart = 0`：Vulkan 侧必须只产生**一次** `vkCmdBindDescriptorSets`，且 `pDynamicOffsets = nullptr`、`descriptorSetCount = 1`；
- 把表内第 2 个槽位的 SRV 换成另一种纹理，着色器采样结果必须变（证明 binding 没写死）；
- 连绑同一张表 100 次，`vkAllocateDescriptorSets` 调用次数应为 1（缓存生效）。

---

## 7. 改动 E：`Map/Unmap` 带范围与一致性

> **落地状态（2026-11-06）：未落地。** 公共签名未变，无 `Invalidate/Flush` 接口；两端后端现状见 7.1。

### 7.1 现状

```cpp
RHIResource.h:354   virtual void* Map()   = 0;
RHIResource.h:358   virtual void   Unmap() = 0;
```

没有 offset / size，也没有 flush / invalidate 的语义。后果：

- Vulkan Readback 堆用 `HOST_VISIBLE | HOST_CACHED` 而不求 `HOST_COHERENT`（`RHIResourceVulKan.cpp:20`，P2-14），GPU 写完 CPU 直接读会拿到旧数据——接口里没有地方表达"读前需要 invalidate"；`BufferVulKan::Map`（`:269`）/ `Unmap`（`:287`）也没有 invalidate/flush 调用；
- D3D12 的 `Unmap` 需要 `D3D12_RANGE`；现在 `Map` 的 `readRange(0,0)`（`RHIResourceD3D12.h:67`）、`Unmap` 的 `writeRange(0,0)`（`:85`）只能传 `(0,0)`（P2-13），语义上等于"CPU 什么都没写"，驱动可能不做回写。

### 7.2 改法

```cpp
// RHIResource.h：改签名，默认参数保证旧调用不变
virtual void* Map(uint64_t Offset = 0, uint64_t Size = 0) = 0;   // Size == 0 → 到结尾
virtual void  Unmap(uint64_t Offset = 0, uint64_t Size = 0) = 0;

// 新增两条非纯虚（默认空实现，D3D12 无需覆盖）
virtual void InvalidateMappedRange(uint64_t Offset, uint64_t Size) {}
virtual void FlushMappedRange(uint64_t Offset, uint64_t Size) {}
```

### 7.3 落地

- **Vulkan**：`BufferVulKan::Map` 记下 `m_MappedOffset/m_MappedSize`；`InvalidateMappedRange` → `vkInvalidateMappedMemoryRanges`，`FlushMappedRange` → `vkFlushMappedMemoryRanges`。`Map` 选内存类型时优先 `HOST_COHERENT`，拿不到才降级并在 debug 下 `WarningCapture`。
- **D3D12**：`Unmap` 用真实范围构造 `D3D12_RANGE{ Offset, Offset+Size }`；`Invalidate/Flush` 保持空实现（UMA/回读堆天然一致）。

### 7.4 验证

- `Map(0, 16)` 后 `Unmap(0, 16)`：D3D12 侧下发的 `D3D12_RANGE` 必须是 `{0,16}` 而不是 `{0,0}`；
- Vulkan Readback：GPU 写 4 字节 → `Map` → 未启 `HOST_COHERENT` 时必须调用过 `vkInvalidateMappedMemoryRanges` 才能读到正确值。

---

## 8. 顶点偏移契约（不改签名，一条注释 + 一处实现）

> **落地状态（2026-11-06）：未落地。** 公共头无偏移语义注释；D3D12 顶点缓冲仍忽略 `pOffsets`（证据见下）。索引缓冲偏移已叠加（`CommandListDX12_InputAssembly.cpp:112,114`），不在本条范围内。

`RHICommandList.h:92` 的 `IASetVertexBuffers(..., const uint64_t* pOffsets)` 在 D3D12 侧被完全忽略：

```cpp
CommandListDX12_InputAssembly.cpp:88-104
view.BufferLocation = ...GetGPUVirtualAddress();   // :97 —— pOffsets 没参与
```

Vulkan 侧 `vkCmdBindVertexBuffers` 原生支持 `pOffsets`，D3D12 需要把它加进 `BufferLocation`。所以：

- `RHICommandList.h:85-92` 补一句注释："`pOffsets[i]` 是字节偏移，必须叠加到该槽位的缓冲起始地址；D3D12 侧没有独立 offset 参数"；
- `CommandListDX12_InputAssembly.cpp:97` 改为 `+ (pOffsets ? pOffsets[i] : 0)`，同时 `:99` 的 `SizeInBytes` 相应减去偏移。

---

## 9. 实施顺序（按"能不能出画面"排）

> 完成度与下一步按 HEAD `88c62df`（2026-11-05 19:35）代码现状重排；编号 M1~M6 沿用原方案，未重排。0c0436c / 88c62df 的 `GraphicsQueueRequirements` 重构不属于 A~F 任何一项，未改变本表完成度。

| 里程碑 | 内容 | 为什么在这个位置 | 完成判据 | 当前完成度（2026-11-06） | 下一步该做什么 |
|---|---|---|---|---|---|
| M1 | `docs` 定稿 + 第 3 节 F（子资源常量、`InitialState`）+ 第 2 节 A 的接口声明 | 纯加字段、不动调用方，先让编译面收敛 | 全仓能编过（`Binary\Debug` 现有产物时间戳为 2026-11-05 19:27:36~38，与 HEAD `88c62df` 源码同源，说明本次复核的基线是编得过的；改完必须重新构建再看结果） | **1/3**：方案文档本次复核更新；F 常量/`InitialState` 未加（`RHIDefinitions.h`）；A 的 `TextureViewDesc` 接口声明未加（`RHIInterface.h`） | 先补公共头增量：`RHI_ALL_SUBRESOURCES` + barrier 默认值/`TextureDesc::InitialState`；`TextureViewDesc` 与四个视图创建签名尾参；C 的两个纯虚与静态/动态状态契约注释 |
| M2 | 第 5 节 D（Vulkan 补 `vkCreateGraphicsPipelines`）+ 第 4 节 C 的里程碑 1 | 没有 graphics PSO，后面全是空中楼阁（P0-1） | `HelloTriangleTest` 能进帧循环不崩 | **0/2**（里程碑口径）：图形 PSO 仍 `return false`（`RHIPipelineStateVulkan.cpp:156-158`），未调 `vkCreateGraphicsPipelines`；显式 `BeginRenderPass` 接口未加。命令侧先行落地：`vkCmdBindPipeline`（`CommandListVK_PipelineState.cpp:31,36`）、`vkCmdSetViewport/Scissor`（`CommandListVK_Rasterizer.cpp:20,36`）已通，但不等同于 PSO 创建侧完成 | 补图形 PSO 创建：rendering formats 取自 `RHIPipelineState.h:45-46`；声明 `VK_DYNAMIC_STATE_VIEWPORT/SCISSOR/PRIMITIVE_TOPOLOGY` + `VkPipelineViewportStateCreateInfo`；再落地 Vulkan 显式 `BeginRenderPass`（里程碑 1，测试一行不改） |
| M3 | 第 4 节 C 的里程碑 2（翻转 `OMSetRenderTargets`）+ 清深度 + pass 内状态 | 决定 P0-2/P0-3/P0-4/P1-5/P1-16/P2-6 一起清 | Vulkan 出第一个三角形 | **0/3**：`OMSetRenderTargets` 仍隐式开 pass；`EndDynamicRendering` 不清缓存（`CommandListVK_OutputMerger.cpp:104-111`）；两端 `ClearDepthStencilView` 空（VK `:36-38`、DX12 `CommandListDX12_Clear.cpp:14-17`）；IA 三函数空（`CommandListVK_InputAssembly.cpp:10-12,14-16,18-20`） | IA 顶点/索引/拓扑绑定先行（出三角形的硬前提）；`EndDynamicRendering` 清 `m_CurrentRenderTargets/m_CurrentRenderArea`；两端实现清深度；再翻转 `OMSetRenderTargets` 为只登记附件 |
| M4 | 第 6 节 B（表级 set）+ 根描述符 + 第 2 节 A 的 Vulkan 纹理视图 | 依赖 M3 的 pass 内绑定顺序 | 一个带 1 CB + 1 纹理的 shader 能跑 | **0/3**：表级 set 未加（`DescriptorHeapVulKan.cpp:170-213` 仍 1-binding/binding 0）；root CBV/SRV/UAV 六函数 TODO；两端纹理/采样器视图未建 | 表级 set 缓存与分配；根描述符 1-binding set 绑定；A 的 Vulkan/D3D12 纹理视图与采样器落地 |
| M5 | 第 7 节 E（Map 范围 + invalidate） | 只影响回读路径，与渲染解耦 | Readback 用例通过 | **0/2**：`Map()/Unmap()` 无范围参数（`RHIResource.h:354,358`）；无 `Invalidate/Flush`；D3D12 仍 `(0,0)`（`RHIResourceD3D12.h:67,85`） | 公共签名加 offset/size 默认参数与两条非纯虚；D3D12 填真实 `D3D12_RANGE`；Vulkan readback 路径补 invalidate |
| M6 | 第 8 节顶点偏移、D3D12 的 `ClearDepthStencilView` | 收尾一致性 | 两端同输入同输出 | **0/2**：DX12 顶点缓冲忽略 `pOffsets`（`CommandListDX12_InputAssembly.cpp:97,99`）；DX12 清深度空（`CommandListDX12_Clear.cpp:14-17`） | 顶点偏移叠加到 `BufferLocation` 并相应缩减 `SizeInBytes`；D3D12 `ClearDepthStencilView` 落地 |

**重排后的当前顺序**（与代码状态自洽）：
1. M1 剩余公共头增量（F 常量 + A 接口声明 + C 纯虚/契约注释）；
2. M2：Vulkan 图形 PSO 创建（含 dynamic state 声明与 viewport/scissor count）+ 显式 pass 里程碑 1；
3. M3：IA 三函数绑定 → pass 缓存清理 → 两端清深度 → 翻转 `OMSetRenderTargets`；
4. M4：B 表级 set + 根描述符 → 两端纹理/采样器视图；
5. M5：Map 范围 + invalidate/flush；
6. M6：顶点偏移与 D3D12 清深度收尾。

依赖关系：M1 接口定稿 → M2 PSO/pass 骨架 → M3 绑定与 pass 收口 → M4 描述符与视图 是硬串行；M5/M6 可在 M3 之后随时插入。

---

## 10. 影响面与兼容

| 改动 | 是否改签名 | 现有调用方要不要动 | 说明 |
|---|---|---|---|
| A 视图 desc | 加默认尾参 | **不用** | 默认 `TextureViewDesc{}` 等价旧行为 |
| C 新增 pass | 新增纯虚 | **不用**（里程碑 1） | 里程碑 2 才让 `HelloTriangleTest.cpp:298-301` 换成 `BeginRenderPass` |
| D 动态状态 | 不改 | 不用 | 纯契约 + Vulkan PSO |
| E Map 范围 | 加默认尾参 | 不用 | `Map()` / `Unmap()` 旧写法仍合法 |
| F 常量 / 默认值 | 默认值改变 | 可能 | 原来依赖 `Subresource = 0` 的地方语义会变；**仓库里目前 `HelloTriangleTest.cpp:291` 与 `:327` 两处显式写 `0`**，不受影响 |
| B 表级 set | 不改 | 不用 | 纯 Vulkan 内部 |

新加的两个纯虚函数（`BeginRenderPass` / `EndRenderPass`）会让 `CommandListVulKan`（`RHICommandListVulKan.h:77-79` 的 OutputMerger 声明区）与 `CommandListD3D12`（`RHICommandListD3D12.h:62-64` 同位置）各多两个声明；**两个后端必须同时补**，否则 `RHI` 模块编不过。

---

## 11. 明确不做（避免下一轮又讨论一遍）

1. **不把 `set` / `binding` / `VkImageLayout` 放进 `RHI/Public`**。它们是 Vulkan 的寻址方式，进公共头等于强制 D3D12 也承受一遍。
2. **不重写 `state → layout` 映射**。`CommandListVK_ResourceBarrier.cpp:9-92` 已经有一套，改它没有收益；只在 aspect（`:95-103`）和首用（`:141`）两处补。
3. **不改 4 个 C 导出与 `GetRHIType()` 回读校验**。模块边界是这套代码里少有的干净部分。
4. **不做 bindless / 描述符索引堆**。当前 `RHIDescriptorHandle` 只有 `{Type, Index}`（`RHIResource.h:111-163`），改成 bindless 会把两个后端一起拖进大重构，与本轮目标（先出画面）无关。
5. **不为 Vulkan 单独开一套 `RHICommandList` 子类**。那会把后端差异漏到上层调用方，比现在的缺口更难维护。

---

## 12. 验收清单

- [ ] `Engine/Source/Runtime` 全量可编（`CMakeLists.txt` 里每个 target 都产出对应 DLL）。现状：产物在 `Binary\Debug`，含 `Magic-RHI.dll`（5,107,200 B）/ `Magic-Core.dll`（1,090,048 B）/ `Magic-D3D12RHI.dll`（1,218,048 B）/ `Magic-VulKanRHI.dll`（1,396,736 B）与 `HelloTriangleTest.exe` / `TestSwapChain.exe` / `TestShaderCompile.exe`，时间戳均为 2026-11-05 19:27:36~38；而全部源文件最后修改时间为 19:27:20、HEAD 提交于 19:35，故这批产物对应的正是与本文同一份源码，Debug 全量链接成功。另有 `VkLayer_khronos_validation.dll`（2026-10-29，可用校验层）但本轮未启用。**未复验的只剩运行时行为**：出图、校验层合法性、内存一致性仍只到代码推演为止
- [x] `RHIDescriptorHandle` 仍是 `{Type, Index}`（`RHIResource.h:111-163`），公共头里搜不到 `Vk` / `vk` / `D3D12_` 句柄或布局类型。**此前记录的两个例外已随 0c0436c 消失**：`_Vk` 字段全部下沉到 `VulKanRHI/Internal` 的 `GraphicsQueueRequirementsVulKan`（`RHICommandListVulKan.h:158-170`），公共头 `GraphicsQueueRequirements`（`RHIDefinitions.h:29-35`）只剩 `GetType()`，本次**不新增**例外。注：`RHIType` 枚举名里的 `D3D12` 是平台标识，不属句柄/布局泄漏；该枚举已从 `RHIPlatformDetection.h` 迁到 `Common/RHITypes.h:13`（0c0436c），改公共头时别在两个头各留一份
- [ ] `SetGraphicsRootDescriptorTable` 只发一次 `vkCmdBindDescriptorSets`，`descriptorSetCount == 1`。卡在 `CommandListVK_RootSignature.cpp:93-95,197-199`：binding 写死 `{Standard,0}`，`offsetInDescriptorsFromTableStart` 被丢弃，表级 set 未建（P1-2）
- [ ] 一张 3 元素 SRV 表能绑定并通过着色器读到第 2、3 个元素。卡在 B 未落地：`DescriptorHeapVulKan.cpp:170-213` 每资源一 set、binding 0、count 1（P1-2、P1-11）
- [ ] 纹理 SRV 的 `VkImageView != VK_NULL_HANDLE`，`D32_FLOAT` 时 aspect 无 `STENCIL`。卡在 `DescriptorViewVulKan.cpp:90-98` 未建 `VkImageView`；aspect 仍由 `CommandListVK_ResourceBarrier.cpp:95-103` 的 `GetAspectMask` 判定，沾深度即 `DEPTH|STENCIL`（P1-18）
- [ ] 连续两帧 pass，第二帧 `m_CurrentRenderArea` 是自身而非上一帧（P2-6 回归）。卡在 `CommandListVK_OutputMerger.cpp:104-111` `EndDynamicRendering` 不清 `m_CurrentRenderTargets` / `m_CurrentRenderArea`
- [ ] `SetPipelineState` 格式不匹配时报错，不静默画黑。卡在 `RHIPipelineStateVulkan.cpp:156-158` 图形 PSO 未创建；`CommandListVK_PipelineState.cpp:22-27` 只做 `IsValid()` 校验，未与 `RenderTargetFormats`（`RHIPipelineState.h:45-46`）比对
- [ ] `Map(0,16)/Unmap(0,16)` 在 D3D12 走真实 `D3D12_RANGE`。卡在 `RHIResource.h:354,358` 无 offset/size 参数；`RHIResourceD3D12.h:67,85` 仍传 `(0,0)`（P2-13）
- [ ] `ClearDepthStencilView` 两端都不再是空函数体。卡在 `CommandListVK_Clear.cpp:36-38`（P1-5）、`CommandListDX12_Clear.cpp:14-17`（P1-16）
- [ ] `docs/MagicRHI-问题清单.md` 中被本方案销掉的条目逐条标注为已处理或另行排期：P0-2 / P0-3 / P0-4、P1-1 / P1-2 / P1-3（采样器那半）/ P1-4 / P1-5 / P1-8 / P1-11 / P1-13 / P1-15 / P1-16、P2-6 / P2-9 / P2-13 / P2-14、P1-18（aspect 与首用两处）。清单已于 2026-11-06 同步更新：48 条编号与状态均未变（仍为已修复 2 / 部分修复 1 / 未修复 45），另追加 N6~N8 三条由 `GraphicsQueueRequirements` 重构引入的新发现；标注时仍需区分命令侧与 PSO/创建侧

---

## 13. 风险

> 复核基准 HEAD `88c62df`（2026-11-06）：以下 1~4 均**未消除**，对应改动（C/B/A/接口新增）尚未落地；5、6 两条来自"命令侧先行、PSO 侧未通"的当前组合；本轮新增第 7 条。

1. **`BeginRenderPass` 翻转默认会打破"OMSetRenderTargets 万能"的直觉**（未消除，C 未落地）。现状仍是 `OMSetRenderTargets` 隐式开 dynamic rendering（`CommandListVK_OutputMerger.cpp:92`）。缓解：Vulkan 侧检测到"未 Begin 就 Draw"时 `WarningCapture` 一次并退化旧路径，不要一上来就 throw。
2. **表级 set 缓存与命令池复位**（未消除，B 未落地）。`ResetCommandPoolMemory`（`RHICommandList.h:408`）复位的是命令缓冲，不含 descriptor set；缓存必须挂在 `DescriptorHeapVulKan` 而不是 `CommandListVulKan`，否则复用时 set 已失效。
3. **`TextureViewDesc` 的 mip/layer 组合**（未消除，A 未落地）。`NumMips/NumLayers` 用 `RHI_ALL_SUBRESOURCES` 哨兵时，Vulkan 侧要换 `VK_REMAINING_MIP_LEVELS`，D3D12 侧要换 `-1`（`UINT(-1)`）；写反了就是 P1-18 的翻版。
4. **两个纯虚函数同时加**（未消除，C 未落地）。任何漏改的后端都会在 `RHI` 编译期炸——这属于想要的效果，但要一次性改完再提交。
5. **命令侧先行、PSO 侧未通，容易被误判为整条链路已落地**（本轮新增）。`vkCmdBindPipeline`（`CommandListVK_PipelineState.cpp:31,36`）与 `vkCmdSetViewport/Scissor`（`CommandListVK_Rasterizer.cpp:20,36`）代码已在，但图形 PSO 恒无效（`RHIPipelineStateVulkan.cpp:156-158`），`SetPipelineState` 在 `:23-27` 的 `IsValid()` 校验处早退；审查时不能把"命令已发出"等同于"动态状态生效"。
6. **图形 PSO 补上后动态态 / viewport state 缺失的校验层风险**（本轮新增，由风险 5 转移细化而来）。当前 `RHIPipelineStateVulkan.cpp` 全文件无 `VkDynamicState` / `pDynamicState` / `VkPipelineViewportStateCreateInfo` / `viewportCount` / `scissorCount` / `vkCreateGraphicsPipelines`（grep 0 命中），两个 struct 整体未构造。后续创建图形 PSO 时两类错误要分开防：漏声明 `VIEWPORT/SCISSOR/PRIMITIVE_TOPOLOGY` → 命令侧 `vkCmdSetViewport/Scissor` 触发校验层 VUID 且不生效；缺 `VkPipelineViewportStateCreateInfo` 或 `viewportCount/scissorCount=0` → PSO 创建阶段即校验失败。
7. **公共头已清零，但新交接结构缺虚析构**（本轮新增，来自 0c0436c / 88c62df）。`GraphicsQueueRequirements`（`RHIDefinitions.h:29-35`）有纯虚函数却没有 `virtual ~`，而 `GetGraphicsQueueRequirements()` 以 `std::unique_ptr<GraphicsQueueRequirements>` 返回派生对象（`RHIInterface.h:329`），经基类指针析构属 UB（问题清单 N6）。下一轮若继续沿用"中性基类 + 后端 Internal 派生"这个模式（例如把 PSO 描述、render pass 描述也这么传），先补虚析构，否则同一处 UB 会被原样复制到新接口上。
