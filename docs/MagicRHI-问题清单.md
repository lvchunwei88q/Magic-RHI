# MagicRHI 问题清单（D3D12 / Vulkan 双后端）

> 审查日期：2026-11-04
> 审查方式：**静态代码走查**（RHI 核心 + D3D12RHI + VulKanRHI 全部关键实现），**未构建、未运行**。
> 版本说明：本文已合并外部旧缺陷文档《代码缺陷.md》的 28 条，并**逐条回代码核对**（判定见附录 A）：**25 条确认为本文遗漏并已收录**（其中第 15、16 条合并为 P2-17），2 条与本文重复，1 条描述有误（旧文档把已实现的 `OMSetRenderTargets` / `ClearRenderTargetView` 判为空操作）。旧文档第 2 条的 PSO 行号也偏移了 2 行（应为 `:158`，非 `:156`）。
> 仓库现状：`out/` 下只有 CMake 编译器探测产物，没有生成过任何 `RHI_D3D12.dll` / `RHI_Vulkan.dll` / 测试 exe，因此下文所有结论均为代码层证据，不含运行时证据。
> 引用格式：`文件:行`，路径相对于仓库根 `D:\C++Project\Magic-RHI`。

---

## 0. 一句话结论

接口是按 D3D12 的语义建模的：**D3D12 后端是透传方，Vulkan 后端是翻译方**。D3D12 的每帧链路完整可跑（状态类参数全是默认值）；Vulkan 的**图形管线是断的**——PSO 从未创建、`vkCmdBindPipeline` 从未发出、IA / 视口 / 裁剪全为空实现，`vkCmdDraw` 会在没有管线绑定的情况下发出，而且不报错。合并旧文档后新增的一批问题集中在**更底层**：两个后端的描述符视图都只分配槽位不建 view、缓冲区初始化时按对齐后尺寸越界读、关闭时先销毁描述符池再等 GPU、`Core` 层的 JobSystem 会遗弃排队任务 — 这些在「能出图」之后仍会继续咬人。

---

## 1. 总览

共 **48 条**：P0 阻塞 4 条（Vulkan 图形链路完全断开）、P1 20 条（功能缺失与正确性，含内存安全与 D3D12 侧错误）、P2 19 条（生命周期 / 并发 / 性能）、P3 5 条（构建与代码卫生）。

| 编号 | 级别 | 位置 | 问题 | 后果 |
|---|---|---|---|---|
| P0-1 | 阻塞 | `VulKanRHI/Private/RHIPipelineStateVulkan.cpp:158` | 图形 PSO 填完 shader stage 后直接 `return false`，从未调用 `vkCreateGraphicsPipelines` | 上层拿到 `nullptr`，HelloTriangle 断言必挂 |
| P0-2 | 阻塞 | `VulKanRHI/Private/CommandList/CommandListVK_PipelineState.cpp:22` | `SetPipelineState` 只有 `// TODO` | 无 `vkCmdBindPipeline`，`vkCmdDraw` 无管线 |
| P0-3 | 阻塞 | `VulKanRHI/Private/CommandList/CommandListVK_InputAssembly.cpp:10,14,18` | `IASetPrimitiveTopology` / `IASetVertexBuffers` / `IASetIndexBuffer` 全空 | 无顶点/索引缓冲绑定 |
| P0-4 | 阻塞 | `VulKanRHI/Private/CommandList/CommandListVK_Rasterizer.cpp:5,10` | `RSSetViewports` / `RSSetScissorRects` 只有 `// TODO` | 无 `vkCmdSetViewport/Scissor`，视口为 0 |
| P1-1 | 功能缺失 | `CommandListVK_RootSignature.cpp:112,121,130,216,225,234` | 六个 root CBV/SRV/UAV 绑定全 `// TODO` | 只有 descriptor table + push constant 生效 |
| P1-2 | 逻辑错误 | `CommandListVK_RootSignature.cpp:94,198` | 绑 descriptor set 时把 heap 写死成 `{Standard,0}`，且忽略 `offsetInDescriptorsFromTableStart` | 多表/带偏移时绑到同一份错误 set |
| P1-3 | 功能缺失 | `VulKanRHI/Private/RHIResourceVulKan.cpp:112,115` | `CreateSamplerState` 直接 `return nullptr`，`DeleteSamplerState` 空 | 采样器永不可用，写 descriptor 必带空指针 |
| P1-4 | 功能缺失 | `CommandListVK_OutputMerger.cpp:113,118` | `OMSetBlendState` / `OMSetDepthStencilState` 空 | VK 侧混合/深度状态不可配 |
| P1-5 | 功能缺失 | `CommandListVK_Clear.cpp:36` | `ClearDepthStencilView` 空函数体 | 深度/模板无法 clear |
| P1-6 | 功能缺失 | `D3D12RHI/Private/RHIPipelineStateD3D12.cpp:24,34,44` | `ConvertBlendState` / `ConvertRasterizerState` / `ConvertDepthStencilState` 三个函数体都是 `// TODO` | PSO 里 blend/raster/depth 全为默认值（调用点 `:257,259,260`） |
| P1-7 | 逻辑错误 | `RHIPipelineStateD3D12.cpp:272-273` | `PrimitiveTopologyType` 写死 `D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE` | 点/线/细分拓扑无法用 |
| P1-8 | 功能缺失 | `D3D12RHI/Private/CommandList/CommandListDX12_OutputMerger.cpp:32,37` | `OMSetBlendState` / `OMSetDepthStencilState` 空 | D3D12 侧同样不可动态配状态 |
| P1-9 | 内存安全 | `VulKanRHI/Private/RHIResourceVulKan.cpp:126`、`D3D12RHI/Private/RHIResourceD3D12.cpp:177,186` | CB 尺寸按 256 对齐后，`memcpy` 仍用对齐后大小拷贝 `InitialData` | 调用方只提供原尺寸缓冲区 → **越界读**（两个后端都有） |
| P1-10 | 逻辑错误 | `RHIRootSignatureVulKan.cpp:327,342`、`CommandListVK_RootSignature.cpp:149` | push-constant range 索引在合并前保存、合并后使用 | 多个 Constants 参数时偏移/范围错位，常量互相覆盖 |
| P1-11 | 功能缺失 | `VulKanRHI/Private/DescriptorViewVulKan.cpp:90,100` | 纹理/采样器描述符入口只 `Allocate()` 一个槽位，不建 `VkImageView`/`VkSampler`、不写 descriptor | 句柄「看起来有效」但无法绑到 GPU |
| P1-12 | 逻辑错误 | `RHIResourceVulKan.cpp:37`、`DescriptorHeapVulKan.cpp:275` | Buffer SRV 写成 `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER`，但创建时只声明了 `STORAGE_TEXEL_BUFFER` | 违反 usage/descriptor 匹配，校验层报错、采样读不到 |
| P1-13 | 功能缺失 | `D3D12RHI/Private/DescriptorViewD3D12.cpp:91,101` | 纹理描述符从 **sampler 堆** 分配，`Type` 被忽略，不创建 SRV/UAV；`SamplerState` 也只占槽不建 sampler | 句柄属于错误的堆类型，必然绑不上 |
| P1-14 | 正确性 | `CommandListDX12_Resource.cpp:5,9` | `CopyResource` 对 `RHIResource*` 做 `static_cast<BufferD3D12*>`，无类型校验 | 传纹理进去即 UB，访问错误的资源指针 |
| P1-15 | 逻辑错误 | `CommandListDX12_InputAssembly.cpp:88,97` | `IASetVertexBuffers` 完全忽略 `pOffsets`，`SizeInBytes` 也用整缓冲大小 | 非零偏移时读到错误顶点数据 |
| P1-16 | 功能缺失 | `D3D12RHI/Private/CommandList/CommandListDX12_Clear.cpp:14` | `ClearDepthStencilView` 空函数体（只有 `// TODO`） | 深度/模板无法 clear（VK 侧同样空，见 P1-5） |
| P1-17 | 逻辑错误 | `RHIResourceD3D12.cpp:173` | 对齐判断用 `flag == BufferBindFlag::ConstantBuffer`（相等），组合标志会漏 | CB + 其他标志组合时不对齐，CBV 无效；VK 侧用的是 `EnumHasAnyFlags`，是对的 |
| P1-18 | 正确性 | `CommandListVK_ResourceBarrier.cpp:95,100` | `GetAspectMask` 只要涉及深度就固定返回 `DEPTH_BIT|STENCIL_BIT` | D16/D32 等无模板格式的 barrier 非法 |
| P1-19 | 正确性 | `RHIShaderCompiler/RHISPIRVGenerationReflection.cpp:156,163-170` | push constant 被当作普通 descriptor 读 `DescriptorSet`/`Binding` decoration | 缺省拿到 0/0，混进 `resourcesBySet[0]` 与真实资源冲突 |
| P1-20 | 崩溃风险 | `RHIShaderCompiler/RHICompilePipeline.cpp:47,56,63` | `controller` 判空只在 `#if RHI_ENABLE_DEBUG_INFO` 内，Release 下继续解引用 | Release 构建空指针崩溃 |
| P2-1 | 性能/语义 | `VulKanRHI/Internal/RHICommandListVulKan.h:219`、`RHICommandListVulKan.cpp:241` | 只有 1 个 `m_inFlightFence`，3 个后备缓冲被串行化 | 实际只有 1 帧在飞，CPU/GPU 无法重叠 |
| P2-2 | 资源管理 | `RHICommandListVulKan.cpp:191-198,225,252` | 信号量按 `imageIndex` 建，fence 只有 1 个；resize 时旧信号量不销毁 | 同步模型自相矛盾；窗口来回改尺寸会漏信号量 |
| P2-3 | 语义隐患 | `D3D12RHI/Private/RHICommandListD3D12.cpp:229,240,251` | 提交时写 `m_FrameFenceValues[m_CurrentFrameIndex]`，但槽号要到 `EndFrame` 才更新 | 记进数组的其实是上一帧槽位，能跑但语义错 |
| P2-4 | 性能 | `RHIResourceD3D12.cpp:261`、`RHIResourceVulKan.cpp:202` | `CreateBuffer` 内部为了上传初始数据直接 `WaitForGPU` | 加载期完全同步，无异步上传 |
| P2-5 | 性能/扩展性 | `RHIResourceVulKan.cpp:94` | 每个 buffer 一次 `vkAllocateMemory` | 无 sub-allocation，资源数量受 `memoryTypeCount` 限制 |
| P2-6 | 逻辑错误 | `CommandListVK_OutputMerger.cpp:104-111` | `EndDynamicRendering` 不清 `m_CurrentRenderTargets` | 一帧内二次 `OMSetRenderTargets` 后，`ClearRenderTargetView` 的附件索引指向旧 RTV |
| P2-7 | 健壮性 | `VulkanRHI/Private/SwapChainVulkan.cpp`（`OUT_OF_DATE` 分支） | 交换链无重建路径，只打警告 | 改窗口尺寸/全屏切换后会持续 OUT_OF_DATE |
| P2-8 | 性能 | `RHICommandListVulKan.cpp:95-111` | 每次 `CreateCommandList` 都 `vkAllocateCommandBuffers(1)`，无复用 | 每帧新建 command buffer，长期碎片化 |
| P2-9 | 资源泄漏 | `VulKanRHI/Private/DescriptorHeapVulKan.cpp:103-117` | `Free` 只清本地记录，全程没有 `vkFreeDescriptorSets` / `vkResetDescriptorPool` | 反复分配/释放持续消耗池，最终耗尽 |
| P2-10 | 生命周期 | `RHIVulKan.cpp:883-905` | `Shutdown` 先 reset 4 个描述符堆，之后才 `vkDeviceWaitIdle` | 已提交命令仍引用这些 set 时销毁，违反 Vulkan 生命周期 |
| P2-11 | 生命周期 | `D3D12RHI/Private/RHID3D12.cpp:275-285` | `Shutdown` 先 reset 4 个堆，队列析构里才 `WaitForGPU`（`RHICommandListD3D12.cpp:203`） | 堆先于 GPU 完成销毁，属同步责任错位 |
| P2-12 | 内存泄漏 | `RHIRootSignatureD3D12.cpp:43,45,140` | `TranslateRanges` 里 `new[]`，序列化后无 `delete[]` | 每建一次根签名泄漏 range 数组 |
| P2-13 | 语义错误 | `D3D12RHI/Internal/RHIResourceD3D12.h:85` | `Unmap` 传 `D3D12_RANGE writeRange(0, 0)`，即「零字节被写」 | 驱动不感知 CPU 写入；将来用于 readback 会读到旧数据 |
| P2-14 | 正确性 | `RHIResourceVulKan.cpp:19-20` | `Readback` 只要求 `HOST_VISIBLE|HOST_CACHED`，允许 non-coherent | GPU 写入后 CPU 读无 `vkInvalidateMappedMemoryRanges`，读到旧数据 |
| P2-15 | 正确性 | `Core/Private/Jobsystem/JobSystem.cpp:69-80,130` | `Uninstall` 置 `running=false` 后 join，队列剩余任务不再执行；`WorkerLoop:162` 直接退出 | 等待这些任务句柄的 `Wait` 永久空转（`yield` 死循环） |
| P2-16 | 死锁 | `JobSystem.cpp:100-106` | `ScheduleBatch` 持 `queueMutex` 时调用用户工厂 `factory(i)` | 工厂内再 `Schedule` 即重入非递归锁 → 死锁 |
| P2-17 | 内存安全 | `Core/Public/Tools/Buffer.h:54,178` | `m_WriteOffset + size`、`m_ReadOffset + count` 先加后比，无溢出检查 | 加法溢出可绕过容量检查，后续 memcpy 越界 |
| P2-18 | 状态不一致 | `Core/Public/Tools/Buffer.h:108-111` | `MoveData` 只 `ShrinkToFit()` + 移走 vector，不重置 `m_WriteOffset`/`m_CurrentSize` | 移出后继续写会写到已空缓冲区的旧偏移 |
| P2-19 | 越界访问 | `D3D12RHI/Private/DescriptorHeapD3D12.cpp:53` | `GetDescriptorHeapView` 校验了句柄类型但没校验 `index < Capacity` | 越界索引访问 `m_Descriptors[index]`（VK 侧 `:94` 有该检查） |
| P3-1 | 构建隐患 | `VulKanRHI/Public/RHIVulKan.h:9` | `VK_USE_PLATFORM_WIN32_KHR` 用 `#define` 写在头里，而 `RHICommandListVulKan.h:4` 先直接 include `<vulkan.h>` | 包含顺序敏感，换 TU 顺序就可能编不过 `vkCreateWin32SurfaceKHR` |
| P3-2 | 可维护性 | `CommandListVK_*` 全体 | 失败路径混用「静默 `return`」和 `ThrowErrorMessage` | 半成品不报错，只能靠校验层兜底 |
| P3-3 | 代码卫生 | `RHIPipelineStateVulkan.cpp:60-157` | graphics 分支构造的 `VkPipelineShaderStageCreateInfo` 全部未被使用即 `return false` | 死代码，易误判「已实现」 |
| P3-4 | 测试 | `Tests/HelloTriangleTest/HelloTriangleTest.cpp` | 断言依赖 `graphicsPSO->IsValid()` | 选 Vulkan 后端必挂，且失败点远离真因 |
| P3-5 | 测试 | `Tests/TestSwapChain/TestSwapChain.cpp:17,86,96` | `WM_DESTROY` 里直接 `return 0`，不发退出消息；循环 `while(true)` 只靠 5 秒超时退出 | 用户关窗后仍向已销毁窗口的交换链提交，直到超时 |

---

## 2. 详细条目

### P0-1 图形 PSO 从未创建

**位置**：`Engine/Source/Runtime/VulKanRHI/Private/RHIPipelineStateVulkan.cpp:158`

**现象**：`Initialize(Device*, const GraphicsPipelineStateDesc&)` 从 `:20` 开始，逐段校验 device / rootSignature / VS / PS / GS / HS / DS 并填好 `VkPipelineShaderStageCreateInfo`，最后一句是

```cpp
        return false;   // :158 —— 前面全是校验，从来没有 vkCreateGraphicsPipelines
```

对比计算管线 `:224` 是真的调了 `vkCreateComputePipelines`，所以 **Vulkan 侧只有 compute 能用**。

**影响**：`DeviceVulKan::CreateGraphicsPipelineState` 拿不到有效的 `VkPipeline`，HelloTriangle 在 PSO 有效性断言处失败；这也是整个 VK 图形链的唯一根断点。

**建议修法**：
1. 按 `desc` 组 `VkGraphicsPipelineCreateInfo`：
   - `pVertexInputState` 由 `desc.pInputLayout` 转 `VkVertexInputBindingDescription` + `VkVertexInputAttributeDescription`；
   - `pInputAssemblyState` 的 `topology` 与 `pRasterizationState` 的 `polygonMode/cullMode/frontFace` 由 `desc` 转；
   - `pViewportState` 里 `viewportCount/scissorCount=1`，并在 `VkPipelineDynamicStateCreateInfo` 声明 `VK_DYNAMIC_STATE_VIEWPORT`、`VK_DYNAMIC_STATE_SCISSOR`（对应 P0-4 的 `vkCmdSet*`，否则 dynamic state 不生效）；
   - `pColorBlendState` 按 `desc.PipelineRenderTargetBlendDesc` 生成 `VkPipelineColorBlendAttachmentState`（可先只支持一档简单的 alpha blend，其余留 warning）；
   - `pDepthStencilState` 按 `desc` 生成，enabled 由是否带 DSV 决定；
   - `layout = GetPipelineLayout()`；
   - **关键**：dynamic rendering 必须在 `VkPipelineRenderingCreateInfoKHR`（`pNext`）里给出 `colorAttachmentFormats[]` 与 `depthFormat`，否则校验层直接报错、管线不可用。
2. `vkCreateGraphicsPipelines(...)`，失败时 `ThrowErrorMessage` + `return false`。
3. 建议顺手把 `:60-157` 那批未使用的 stage info 真正接进 `pStages`，消除 P3-3。

**验证**：HelloTriangle 用 `RHIType::VulKan` 跑通到「PSO 非空且 `IsValid()`」；Vulkan 校验层无 error。

---

### P0-2 / P0-3 / P0-4 录制层三个空实现（必须一起修）

**位置**：
- `CommandListVK_PipelineState.cpp:5-23`（`SetPipelineState`，TODO 在 `:22`）
- `CommandListVK_InputAssembly.cpp:10,14,18`
- `CommandListVK_Rasterizer.cpp:5,10`（TODO 在 `:7,12`）

**现象**：四个函数都是「校验一遍参数 → 什么都不做」。`SetPipelineState` 里已经把 `pPipelineState` 的类型和 `stateType` 校验完了，最后一句是 `// TODO: 实现管线状态设置`。

**影响**：`vkCmdDraw*` 会照常发出，但命令流里没有 `vkCmdBindPipeline` / `vkCmdBindVertexBuffers` / `vkCmdBindIndexBuffer` / `vkCmdSetViewport` / `vkCmdSetScissor`。设备能跑完、屏幕什么都没有、错误日志也干净——这类问题最难查。

**建议修法**（改动很小，收益最大）：
```cpp
// SetPipelineState
m_pCommandList... vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, SafeCast<RHIPipelineStateVulkan>(pPipelineState)->GetPipeline());
// 记住当前 PSO，配合 dynamic state 立刻补一次 viewport/scissor
m_pCurrentPSO = pPipelineState;
if (m_CachedViewportCount) vkCmdSetViewport(...);
if (m_CachedScissorCount)  vkCmdSetScissor(...);

// IASetVertexBuffers
std::vector<VkBuffer> bufs(n); std::vector<VkDeviceSize> offs(n);
// BufferVulKan::GetBuffer() / GetSize()
vkCmdBindVertexBuffers(m_CommandBuffer, startSlot, numBuffers, bufs.data(), offs.data());

// IASetIndexBuffer
vkCmdBindIndexBuffer(m_CommandBuffer, SafeCast<BufferVulKan>(pIndexBuffer)->GetBuffer(), offset,
                     format == RHIIndexFormat::UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

// RSSetViewports / RSSetScissorRects
vkCmdSetViewport(m_CommandBuffer, 0, numViewports, conv);   // RHIViewport(x,y,w,h,minDepth,maxDepth)
vkCmdSetScissor (m_CommandBuffer, 0, numRects,    conv);   // RHIRect → VkRect2D{offset, extent}
```

注意：`SetPipelineState` 里要按 `PipelineType` 分 `GRAPHICS` / `COMPUTE` bind point，`Draw` 与 `Dispatch` 用的不是同一个。

**验证**：Vulkan 校验层开启后，跑 HelloTriangle 不应再出现 `VUID-vkCmdDraw-None-*`（未绑管线/未绑顶点缓冲）这类报错。

---

### P1-1 Vulkan 无 root-level CBV/SRV/UAV

**位置**：`CommandListVK_RootSignature.cpp:112,121,130`（graphics）、`:216,225,234`（compute）

**现象**：六个函数体全是 `// TODO`，只留 `(void)` 消除未使用参数警告；debug 模式下靠 `CHECK_BUFFER_TYPE` 宏打一条 warning。文件本身在 `:30-36` 的注释里也承认了这一点。

**为什么值得单独列**：这是**接口与目标 API 结构性不匹配**，不是漏写。D3D12 允许 root signature 里直接放一个 GPU 虚拟地址的 CBV/SRV/UAV，Vulkan 没有这个概念。

**建议修法**（三选一，按改动量排序）：
1. **每 root 参数一个极小 descriptor set**：在 `RHIRootSignatureVulKan` 里为每个 `RootDescriptor` 类型参数建一个 1-binding set layout，绑定时 `vkUpdateDescriptorSets` 写一条再 `vkCmdBindDescriptorSets`。改动最小、最好懂，缺点是每帧多几次 update。
2. **`VK_KHR_push_descriptors`**：直接 `vkCmdPushDescriptorSetKHR`，每帧零分配，最贴近 D3D12 语义。需要查扩展支持并在 device 创建时启用，同时改 `VkPipelineLayoutCreateInfo` 的 `flags |= VK_PIPELINE_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR`。
3. **统一到 descriptor heap/table**：把 root descriptor 在 RHI 层就降级成 table + offset，Vulkan 侧不需要新原语，但会改变上层用法。

> 建议：短期内先在 RHI 层立一条约束「Vulkan 后端只保证 descriptor table + 32bit constants」并在文档里写死，避免这六个函数长期半生效。

**验证**：写一个用 root CBV 传常量的 compute 测试，Vulkan 下能读到正确值。

---

### P1-2 descriptor set 绑定写死了 heap 和 index

**位置**：`CommandListVK_RootSignature.cpp:92-104`（graphics）、`:196-208`（compute）

**现象**：
```cpp
        const DescriptorSetLayoutInfo& layoutInfo = m_pCurrentGraphicsRS->GetDescriptorSetLayout(slot.DescriptorSetIndex);
        const RootParameterDesc& tableParam = m_pCurrentGraphicsRS->GetRootParameterDesc(rootParameterIndex);

        // TODO : Implement this method
        DescriptorSetBinding binding;
        if (!pHeap->GetDescriptorSetBinding({RHIDescriptorHeapType::Standard,0},binding)) return;   // ← 写死类型 + 写死 0
```

`layoutInfo`、`tableParam` 取出来完全没用（编译器会报未使用变量或静默），`offsetInDescriptorsFromTableStart` 也被丢弃。

**影响**：只要一个 PSO 里有**两个 descriptor table**，或者表起点不是 0（典型情况：多个 SRV 连续放在一个堆里），两次绑定都会绑到同一份 set → 采样到错误资源，且不报错。这是比 P1-1 更隐蔽的错误，因为它「看起来绑上了」。

**建议修法**：用 `RootParameterDesc` 里的堆类型（而不是常量 `Standard`），用 `offsetInDescriptorsFromTableStart` 定位到具体 `DescriptorSetBinding`；`layoutInfo` 用于校验 binding 数量是否与 layout 匹配。

**验证**：造一个两张表、第二张表偏移到堆中间位置的测试，Vulkan 下渲染结果应与 D3D12 一致。

---

### P1-3 采样器完全不可用

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:112,115`

```cpp
    std::shared_ptr<RHISamplerState> DeviceVulKan::CreateSamplerState(const SamplerStateDesc& desc)
    {
        return nullptr;
    }
```

**影响**：任何带 `SamplerState` 的 texture descriptor 写进去都是空 `VkSampler`；D3D12 侧 `CreateSamplerState` 正常，所以这是两后端行为不一致，而不只是缺功能。

**建议修法**：`vkCreateSampler`，从 `SamplerStateDesc` 转 `filter`（`min/mag/mipmap`）、`addressMode`（U/V/W）、`maxAnisotropy`、`compareOp`、`borderColor`；`DeleteSamplerState` 里 `vkDestroySampler` 并把 `shared_ptr` 置空。

---

### P1-4 / P1-5 / P1-8 状态类接口两侧都空

`OMSetBlendState` / `OMSetDepthStencilState` 在 D3D12（`CommandListDX12_OutputMerger.cpp:32,37`）与 Vulkan（`CommandListVK_OutputMerger.cpp:113,118`）**都是空的**；`ClearDepthStencilView`（`CommandListVK_Clear.cpp:36`）在 Vulkan 侧是空函数体。

**判断**：这一组属于「接口已定义、实现留空」的半成品。当前 HelloTriangle 不需要它们，所以不明显；一旦要做 3D 场景（深度测试、透明混合）就会立刻撞墙。

**建议修法**：D3D12 的 `OMSetBlendState`/`OMSetDepthStencilState` 本身就只是记录状态（D3D12 里 blend/depth 属于 PSO，命令级只能改 blend factor 和 stencil ref），所以实现应该是「缓存到 `m_CurrentBlendFactor` / `m_CurrentStencilRef`，供后续 `OMSetRenderTargets` 的 `OMSetBlendFactor` 使用」；Vulkan 侧同理，仅缓存 + 断言，或者干脆在接口层标注「不支持的动态状态」。

---

### P1-6 / P1-7 D3D12 状态转换函数是空壳

**位置**：`D3D12RHI/Private/RHIPipelineStateD3D12.cpp:24-52`，调用点在 `:257,259,260`

```cpp
        D3D12_BLEND_DESC ConvertBlendState(RHIBlendState* blendState)
        {
            ...
                // TODO: Blend state data initialization
        }
```

三个函数都只做了「判空 + 返回默认值」，真正的字段转换（`BlendEnable`/`SrcBlend`/`DestBlend`/`BlendOp`、`FillMode`/`CullMode`/`FrontCounterClockwise`/`DepthBias`、`DepthEnable`/`DepthFunc`/`StencilEnable`）全部未做。另外 `:272-273` 把 `PrimitiveTopologyType` 写成常量 `TRIANGLE`。

**影响**：D3D12 能画出三角形，纯粹是因为默认值恰好够用（`BlendEnable=FALSE`、`CullMode=BACK`、`DepthEnable=TRUE`）。一旦上层传非默认的 `RHIBlendState`/`RHIRasterizerState`，**静默失效**——上层以为设了，实际没设。

**建议修法**：按 RHI 枚举 → D3D12 枚举一一映射，注意 `RHIBlendFactor`/`RHICullMode` 的枚举值不要直接 `static_cast`（两边数值顺序通常不同，容易错位，建议用 `switch` 显式映射）；`PrimitiveTopologyType` 从 `desc.PrimitiveTopology` 推导（POINT→POINT、LINE→LINE、TRIANGLE→TRIANGLE、PATCH→PATCH）。

**验证**：跑一个「关掉背面剔除 + 开 alpha blend」的测试用例，两个后端出图应一致。

---

### P1-9 常量缓冲区初始化越界读（两个后端同源）

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:126,144`、`D3D12RHI/Private/RHIResourceD3D12.cpp:177,186`

**现象**：`CreateBuffer` 先按常量缓冲区规则把 `desc.SizeInBytes` 对齐到 256（`RHIResourceVulKan.cpp:126` / `RHIResourceD3D12.cpp:177`），这一步改的是**引用传入的调用方 desc**；随后初始化数据时又用这个**已放大的**尺寸做 `memcpy(mappedData, desc.InitialData, desc.SizeInBytes)`（`:144` / `:186`）。调用方给的 `InitialData` 只按原尺寸分配，于是最多多读 255 字节。

**影响**：堆越界读（常读到无关数据），理论上可触发崩溃；UB，调试运行时不保证复现。

**建议修法**：进入函数先保存 `const uint64_t providedBytes = desc.SizeInBytes;`，对齐只用于**资源创建尺寸**；`memcpy` 用 `providedBytes`，对齐出来的尾部按需 `memset` 0（否则着色器可能读到垃圾）。这两个后端都要改。

**验证**：开 ASan（MSVC 用 `/fsanitize=address`）创建 1 个 CB，`InitialData` 用刚好填满的栈数组，确认不再报越界。

---

### P1-10 push-constant 合并后仍用旧索引

**位置**：`VulKanRHI/Private/RHIRootSignatureVulKan.cpp:324-330,342`；消费端 `CommandListVK_RootSignature.cpp:149`

**现象**：每个 `RootParameterType::Constants` 参数在**合并前**把自己的下标存进 slot：
```cpp
    const uint32_t pushRangeIndex = static_cast<uint32_t>(temp_pushConstantRanges.size());
    temp_pushConstantRanges.push_back(pushRange);
    slot.PushRangeIndex = pushRangeIndex;              // :327 记录的是「临时列表」下标
```
随后 `:342` 执行 `m_PushConstantRanges = MergePushConstantRanges(temp_pushConstantRanges);`，把同 stageFlags 的 range 合并。绑定时却拿旧下标去查新数组：`GetPushConstantRange(slot.PushRangeIndex)`。

**影响**：只要有两个及以上的 Constants 参数（或合并让数组变短），就会取到错误的 range —— `offset`/`size`/`stageFlags` 全错，`vkCmdPushConstants` 写到错误偏移甚至越界；同时 `:321,329` 每个参数都从 `offset=0` 起算，合并后本应错开却都写 0，互相覆盖。

**建议修法**：`MergePushConstantRanges` 返回时同步产出「旧下标 →（新下标, 新内偏移）」映射，回写到每个 slot 的 `PushRangeIndex` / `PushConstantBaseOffset32`；不要在 slot 里长期保存临时下标。

**验证**：一个 PSO 带 2 个 Constants 参数（stageFlags 相同触发合并），Vulkan 下两个常量都能读到正确值。

---

### P1-11 纹理/采样器描述符只有槽位、没有真正的 view

**位置**：`VulKanRHI/Private/DescriptorViewVulKan.cpp:90-98,100-108`

**现象**：
```cpp
    RHIDescriptorHandle DeviceVulKan::CreateStandardHeapDescriptorView(RHITexture* Texture, DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull()) return RHIDescriptorHandle();
        return m_pStandardHeap->Allocate();     // 只分配槽位，Texture / Type 完全没用
    }
```
采样器入口同样只 `Allocate()`。对比同文件里 buffer 版本（`:70` 等）确实 `new UnorderedAccessViewVulKan(...)` 并写 descriptor。

**影响**：纹理 SRV/UAV 的句柄是「有效外观的空槽」——`GetDescriptorHeapView` 能取到槽，但里面 `ViewType == RRT_None`，写到 set 里就是空 `imageView`。这与 P1-3（`CreateSamplerState` 返回 `nullptr`）叠加，Vulkan 侧目前**完全没有可用的纹理采样路径**。

**建议修法**：创建 `VkImageViewCreateInfo`（`viewType` 由 `desc` 的 1D/2D/3D/Cube 决定、`format` 取 `VkFormat`、`subresourceRange` 按 mip/layer）→ `vkCreateImageView` → 包成 `ShaderResourceViewVulKan` → `SetSRVDescriptor(handle, view)`；采样器同理 `vkCreateSampler`。失败时 `Free(handle)` 并返回空句柄。

---

### P1-12 Buffer SRV 的 descriptor type 与 usage 不匹配

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:37-38`、`DescriptorHeapVulKan.cpp:275`

```cpp
    if (EnumHasAnyFlags(bindFlags, BufferBindFlag::ShaderResource))
        usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;  // 没有 STORAGE_BUFFER_BIT
...
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;   // DescriptorHeapVulKan.cpp:275
```

**影响**：`VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` 要求 buffer 带 `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`。校验层会直接报 `VUID-VkWriteDescriptorSet-descriptorType-00330` 一类错误；不开校验层则是「绑上了但读不到/读垃圾」。

**建议修法**：二选一——(a) 把 SRV 写成 `VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER` 并配 `VkBufferView`；(b) 给 `ShaderResource` 补 `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT` 保持 `STORAGE_BUFFER` 语义。选哪个取决于 RHI 层约定的是 structured buffer 还是 typed buffer，建议在 `BufferBindFlag` 注释里写死。

---

### P1-13 / P1-14 / P1-15 / P1-16 / P1-17 D3D12 侧的五处正确性问题

| 编号 | 位置 | 现象 | 建议 |
|---|---|---|---|
| P1-13 | `DescriptorViewD3D12.cpp:91,98` | `CreateStandardHeapDescriptorView(RHITexture*, Type)` 从 `m_pSamplerHeap` 分配，`Type` 未使用；`CreateSamplerHeapDescriptorView:101` 也只占槽不建 sampler | 按 `Type` 选堆并 `CreateShaderResourceView`/`CreateUnorderedAccessView`；采样器走 `CreateSampler` |
| P1-14 | `CommandListDX12_Resource.cpp:5,9` | `static_cast<BufferD3D12*>(pDstResource)` 无校验 | 先判 `RHIResourceType` 分派到 Buffer/Texture，或在入口拒绝 |
| P1-15 | `CommandListDX12_InputAssembly.cpp:88,97` | `view.BufferLocation` 用基址、`SizeInBytes` 用整缓冲，`pOffsets` 未使用 | `Location = 基址 + pOffsets[i]`、`SizeInBytes = 总大小 - pOffsets[i]` |
| P1-16 | `CommandListDX12_Clear.cpp:14` | `ClearDepthStencilView` 只有 `// TODO` | 调 `ClearDepthStencilView(dsv, flags, depth, stencil, 0, nullptr)`；`RHIClearFlags` 映射到 `D3D12_CLEAR_FLAG_DEPTH/STENCIL` |
| P1-17 | `RHIResourceD3D12.cpp:173` | `flag == BufferBindFlag::ConstantBuffer` 用相等 | 改 `EnumHasAnyFlags(flag, BufferBindFlag::ConstantBuffer)`（VK 侧 `RHIResourceVulKan.cpp:37` 已是正确写法，可对照） |

---

### P1-18 深度屏障的 aspect 恒带 stencil

**位置**：`VulKanRHI/Private/CommandList/CommandListVK_ResourceBarrier.cpp:95-103`

```cpp
    if (涉及 DepthWrite/DepthRead)
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;  // 不看真实 format
```

**影响**：`VK_FORMAT_D16_UNORM`、`VK_FORMAT_D32_SFLOAT`（无模板）用 stencil aspect 做 barrier 是非法的，校验层报 `VUID-VkImageMemoryBarrier-image-03319`；不开校验层则是偶发 undefined behavior。

**建议修法**：把 `VkFormat` 放进 `TextureVulKan`，在 `GetAspectMask` 里按 format 判断（`D24S8`/`D32S8` 才带 stencil）。注意参数只有状态、没有资源，建议改成传 `TextureVulKan*`（调用处 `:140` 已拿到纹理指针，顺手可取 format）。

---

### P1-19 SPIR-V 反射把 push constant 当普通 descriptor

**位置**：`Engine/Source/Runtime/RHI/Private/RHIShaderCompiler/RHISPIRVGenerationReflection.cpp:156-171`

**现象**：push constant 循环里读 `DecorationDescriptorSet` / `DecorationBinding` 并 `reflection.resourcesBySet[res.set].push_back(res)`。push constant 按 SPIR-V 规范**没有** set/binding decoration，`get_decoration` 落到默认 0 → 被塞进 `set 0` 里、和真实资源撞号。

**影响**：上层按 `set/binding` 找资源时会命中错的条目（比如把 CB 认成 push constant），D3D12 侧生成的根签名参数也会多出一组假资源。

**建议修法**：push constant 走独立类别（`reflection.pushConstants`），只取 `type_id` 的 `declared_struct_size` 和成员偏移，不进 `resourcesBySet`。

---

### P1-20 Release 下编译器状态机的空指针

**位置**：`Engine/Source/Runtime/RHI/Private/RHIShaderCompiler/RHICompilePipeline.cpp:47-56`（`BeginCompiler`）、`:63`（`EndCompiler` 同构）

```cpp
    if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
        ThrowErrorMessage("Compiler context controller is null");
#endif
    }
    if(controller->GetCompilerPipelineState() != ...)   // :56 Release 下 controller 可能就是 nullptr
```

**影响**：Debug 能报错，Release 直接空指针解引用崩溃——恰好是「只在你给用户的发布版里崩」的类型。

**建议修法**：`if (!controller) { ThrowErrorMessage(...); return false/return; }`，把 `return` 移到 `#if` 外面（`ThrowErrorMessage` 本身可以留在条件编译里）。全项目搜一遍同构写法（`if (!x) { #if DEBUG ThrowErrorMessage #endif }` 后面继续用 `x`）。

**验证**：Release 构建 + 故意不初始化 compiler context，确认走 return 而不是崩。

---

### P2-1 / P2-2 Vulkan 帧同步只有 1 个 fence

**位置**：`VulKanRHI/Internal/RHICommandListVulKan.h:219`（`VkFence m_inFlightFence`，创建于 `:167` 且带 `SIGNALED` 位）、`RHICommandListVulKan.cpp:238-283`

**现象**：`BeginFrame` 只 `vkWaitForFences(1, &m_inFlightFence, ...)`，`ExecuteCommandLists` 用同一个 fence 做 `vkQueueSubmit` 的完成信号。交换链有 3 张图，`m_RenderFinishedSemaphores` 也按 `imageIndex` 建了 3 个，但 **CPU 侧只有 1 个 fence**，于是第 2、3 帧无法与第 1 帧重叠。

顺带：`:191-198` 在 swapchain 重建导致 `SwapChainBackBufferCount` 变化时 `resize` 数组，旧信号量没有 `vkDestroySemaphore` 就被覆盖了。

**影响**：三缓冲退化成单缓冲（延迟上升、跑不满 GPU）；`RHI_MULTI_BUFFERING=3` 这个配置在 Vulkan 路径上等于没生效。

**建议修法**：改成 `std::vector<VkFence> m_inFlightFences`（大小 = 后备缓冲数），`BeginFrame` 等 `m_inFlightFences[m_CurrentFrameIndex]`，提交时 signal 同一个，`EndFrame` 里 `(m_CurrentFrameIndex+1) % N`。这与 D3D12 现有的 `m_FrameFenceValues[]` 思路一致，两边可以对齐。

**验证**：抓一帧的时间线（PIX / RenderDoc 或自己计时），确认 3 帧可以同时在飞。

---

### P2-3 D3D12 fence 记账时序错位

**位置**：`D3D12RHI/Private/RHICommandListD3D12.cpp:229,240,251`

```cpp
    void GraphicsCommandQueueD3D12::ExecuteCommandLists(...)
    {
        ...
        m_FrameFenceValues[m_CurrentFrameIndex] = m_CurrentFenceValue;   // :229 用「上一帧」的槽号
        ...
    }
    void GraphicsCommandQueueD3D12::EndFrame()
    {
        m_CurrentFrameIndex = m_SwapChainInfo.imageIndex;                // :251 这里才换槽号
```

**影响**：能跑（因为每帧都覆盖写），但 `m_FrameFenceValues[i]` 的语义变成「i 槽上一次提交的值」，而 `BeginFrame`（`:240`）读同一个槽——两者刚好错开一帧，一旦以后有人把 `BeginFrame` 改成更严格的等待策略就会变成偶发 hang。

**建议修法**：把 imageIndex 的获取提前到 `BeginFrame`（`AcquireNextImage` 之后立刻 `m_CurrentFrameIndex = imageIndex`），`ExecuteCommandLists` 里再写本帧的 fence 值，语义就顺了。

---

### P2-4 资源创建内部同步等待 GPU

**位置**：`D3D12RHI/Private/RHIResourceD3D12.cpp:261`、`VulKanRHI/Private/RHIResourceVulKan.cpp:202`

两个后端的 `CreateBuffer` 在带 `InitialData` 时，都是「建 staging/upload → 录制拷贝 → 提交 → `WaitForGPU()`」，且这个 `WaitForGPU` 是**进入函数就会阻塞主线程**的。

**影响**：加载期每创建一个大 buffer 就全球串行一次；不能批量上传，也不能和别的加载工作重叠。

**建议修法**：拆两段——`CreateBuffer` 只创建资源并返回一个「上传请求」句柄，真正的拷贝在帧内统一的 upload pass 里批量录进同一个 copy command list，帧末一次性等 fence。这是所有成熟 RHI 的标准做法（staging ring + deferred upload）。

**验证**：创建 100 个 buffer，总耗时从「100 次 GPU 往返」降到「1 次往返」。

---

### P2-5 Vulkan 无 sub-allocation

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:94`（每个 buffer 一次 `vkAllocateMemory`）

**影响**：`vkAllocateMemory` 有设备级上限（常见 4096 左右 `maxMemoryAllocationCount`），大量小 buffer 会直接分配失败；同时每份分配都按 `memoryType` 对齐，显存浪费明显。D3D12 侧用 `CreateCommittedResource` 也是同构问题，但 D3D12 的 committed resource 内部有驱动侧的分配器兜底，Vulkan 没有。

**建议修法**：引入一个按 `memoryTypeIndex` 分桶的线性分配器（每桶一个 `VkDeviceMemory` 大块 + free list），`CreateBuffer` 从桶里切一段偏移，`vkBindBufferMemory(..., offset)`。buffer 销毁时归还到 free list。

---

### P2-6 动态渲染附件列表不清空

**位置**：`CommandListVK_OutputMerger.cpp:104-111`

```cpp
    void CommandListVulKan::EndDynamicRendering()
    {
        ...
        m_pfnCmdEndRenderingKHR(m_CommandBuffer);
        m_IsDynamicRendering = false;   // 没有 m_CurrentRenderTargets.clear()
    }
```

而 `OMSetRenderTargets`（`:45`）是 `push_back` 追加，`ResetFrameCaches`（`RHICommandListVulKan.cpp:173`）才 clear（只在 `BeginRecording` 调）。

**影响**：一帧里调两次 `OMSetRenderTargets`（先画到 GBuffer、再画到 backbuffer），第二次之后 `m_CurrentRenderTargets` 是 `[旧, 新]`；此时 `ClearRenderTargetView`（`CommandListVK_Clear.cpp:16`）按 `==` 找索引，会命中**旧**那个元素的下标，把 clear 打到错误的附件槽（`attachment.colorAttachment = 0`），结果是「clear 了不该 clear 的东西」或清不掉。校验层对这种 `colorAttachment` 越界会报 VUID，但如果数量恰好匹配就不会报。

**建议修法**：`EndDynamicRendering` 里 `m_CurrentRenderTargets.clear(); m_CurrentRenderArea = {};`，让附件列表严格跟随当前 rendering scope 生命周期。

---

### P2-7 / P2-8 交换链与命令缓冲的生命周期

- **P2-7**：`SwapChainVulkan.cpp` 的 `Present` 分支对 `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR` 只打警告，没有重建。而 D3D12 侧同样没有 resize 路径。建议抽出 `RecreateSwapChain(width,height)`：等 GPU 空闲 → 销毁 ImageView/RTV 包装（**不销毁 image 本身**，swapchain 拥有它们）→ `vkCreateSwapchainKHR` 新链 → 重新 `vkGetSwapchainImagesKHR` + 建 view → 把所有 `TextureVulKan` 的 `FirstUseOfSwapChainImage` 复位为 `true`（这一点 `CommandListVK_ResourceBarrier.cpp:141,155-163` 已经在用，重建时必须配合）。
- **P2-8**：`DeviceVulKan::CreateCommandList`（`RHICommandListVulKan.cpp:86-112`）每次 `vkAllocateCommandBuffers(count=1)`，`CommandListVulKan` 析构是 `= default`（`RHICommandListVulKan.h:61`），不做释放。虽然 `CommandPoolVulKan` 析构会 `vkDestroyCommandPool` 连带回收，不会真泄漏，但每帧新建的分配开销和 `RESET_COMMAND_BUFFER_BIT` 已经花掉的代价都白付了。建议按 pool 预分配固定数量 primary buffer 循环使用。

---

### P2-9 ~ P2-19 资源生命周期、并发与边界（补充条目）

- **P2-9 descriptor set 只回收槽位**：`DescriptorHeapVulKan.cpp:109-116` 的注释自己写了「release the pool 时统一释放，这里只置空」，但全局没有任何 `vkResetDescriptorPool` 调用点 → 池里的 set 只增不减。**修法**：定期 `vkResetDescriptorPool`（等 GPU 空闲）或改用 `FREE_DESCRIPTOR_SET` 池 + `vkFreeDescriptorSets`。
- **P2-10 Vulkan 关闭顺序**：`RHIVulKan.cpp:886-889` 先 reset 堆，`:905` 才 `vkDeviceWaitIdle`。**修法**：把 `vkDeviceWaitIdle` 提到所有 `reset()` 之前（或先 reset 队列再 reset 堆）。
- **P2-11 D3D12 关闭顺序**：`RHID3D12.cpp:278-285` 先 reset 堆，队列析构（`RHICommandListD3D12.cpp:203`）才 `WaitForGPU`。**修法**：`Shutdown` 开头对三个队列显式 `WaitForGPU()`，再销毁堆。
- **P2-12 根签名 range 泄漏**：`RHIRootSignatureD3D12.cpp:45` `new D3D12_DESCRIPTOR_RANGE[count]`，全文件无 `delete[]`。**修法**：`std::vector<D3D12_DESCRIPTOR_RANGE>` 或 `unique_ptr<T[]>`，`SerializeRootSignature` 返回后自动释放（注意：D3D12 序列化是立即拷贝的，不持有指针，可以安全释放）。
- **P2-13 Unmap 零写入范围**：`RHIResourceD3D12.h:85` 里 `D3D12_RANGE writeRange(0, 0); m_pResource->Unmap(0, &writeRange);` —— 等于告诉驱动「我一个字节都没写」。**修法**：`Map` 时记录本次映射允许写入的字节数，`Unmap` 传 `D3D12_RANGE(0, written)`；或对 UPLOAD 堆直接传 `nullptr`（表示整块都可能被写）。
- **P2-14 Readback 无非一致失效**：`RHIResourceVulKan.cpp:19-20` 只要求 `HOST_VISIBLE|HOST_CACHED`。**修法**：读之前 `vkInvalidateMappedMemoryRanges(device, 1, &range)`，或在 `FindMemoryType` 里追加 `HOST_COHERENT` 并处理找不到的情况。
- **P2-15 JobSystem 遗弃任务**：`JobSystem.cpp:69-80` 置 `running=false` 后 join；`WorkerLoop:162` 的 `while(running)` 使工作线程最多再跑 1 个任务就退出，剩余队列无人消费。而 `Wait:130` 是 `while(counter>0) { TryRunOne(); else yield(); }` → **无限 yield 空转**（不是 cv 阻塞，CPU 100%）。**修法**：`Uninstall` 里先 `running=false` 阻止新任务入队，再循环 `TryRunOne()` 排空队列，最后 join。
- **P2-16 ScheduleBatch 持锁调工厂**：`JobSystem.cpp:102-106` 在 `lock_guard` 作用域内执行 `factory(i)`。**修法**：先在锁外把 `count` 个 `JobFn` 全部生成到局部 vector，再进锁批量 `push` 并一次性 `notify_all`。
- **P2-17 Buffer 边界加法溢出**：`Buffer.h:54`（`m_WriteOffset + size`）、`:178`（`m_ReadOffset + count`）。**修法**：先判 `offset <= total`，再判 `size <= total - offset`；顺手把 `BinaryReader(const BinaryWrite&)` 的 `m_Data` 深拷贝注释（写的是「共享数据」）改正。
- **P2-18 MoveData 不重置元数据**：`Buffer.h:108-111` 只 `ShrinkToFit()` 再 `std::move`。**修法**：移出后 `m_WriteOffset = 0; m_CurrentSize = 0;`（`m_Data` 已被移空）。
- **P2-19 D3D12 描述符视图无边界检查**：`DescriptorHeapD3D12.cpp:53` 直接 `m_Descriptors[index]`。**修法**：补 `if (index >= Capacity) { ThrowErrorMessage(...); return nullptr; }`，与 VK 侧 `DescriptorHeapVulKan.cpp:94` 对齐。

---

### P3 代码卫生 / 构建

- **P3-1**：`VK_USE_PLATFORM_WIN32_KHR` 定义在 `RHIVulKan.h:9` 的 `#define` 里，而 `RHICommandListVulKan.h:4`、`DescriptorHeapVulKan.h:3`、`RHIResourceVulKan.h:3` 都直接 `#include <vulkan.h>`。目前能编过是因为包含顺序刚好让 `RHIVulKan.h` 先被看到；换个 TU 顺序（或预编译头介入）就可能丢掉 `vkCreateWin32SurfaceKHR` 声明。**建议移到 `VulKanRHI/CMakeLists.txt`**：
  ```cmake
  target_compile_definitions(VulKanRHI PRIVATE VK_USE_PLATFORM_WIN32_KHR VULKANRHI_EXPORTS)
  ```
- **P3-2**：`CommandListVK_*` 里有大量 `if (...) return;` 的静默早退，同时又混用 `ThrowErrorMessage`。静默早退在 release 下永远不报，是这批 P0/P1 能藏这么久的原因。建议统一：debug 下全部走 `ThrowErrorMessage`，release 下至少 `Core::ErrorCapture`。
- **P3-3**：`RHIPipelineStateVulkan.cpp:60-157` 构造的 stage info 未被使用（见 P0-1 建议 3）。
- **P3-4**：测试断言与后端强绑。建议 `HelloTriangleTest` 里按 `RHIType` 分支，Vulkan 未完成时 `GTEST_SKIP()` 并带上原因，避免「一选 vk 就挂且看不出根因」。
- **P3-5**：`Tests/TestSwapChain/TestSwapChain.cpp:17,86,96` —— `WM_DESTROY` 里 `return 0` 不发 `WM_QUIT`；主循环 `while(true)` 唯一退出条件是 5 秒超时（`:96`）。**影响**：用户提前关窗后，循环仍 `AcquireNextImage` 并向已销毁窗口的交换链提交，跑满 5 秒；`RHI_ENABLE_DEBUG_INFO` 下会刷一片错误。**修法**：`WM_DESTROY` 里 `PostQuitMessage(0)`，循环里判 `msg.message == WM_QUIT` 立即 break。

---

## 3. 建议修复顺序

1. **先让它编过**（现状：仓库没有成功构建的产物）。两个 DLL + 三个测试都能生成，是所有后续验证的前提。顺带做 P3-1、P1-20（Release 空指针一编译就暴露）。
2. **内存安全三件套（不依赖图形链路，可以现在就修）**：P1-9（CB 对齐后越界读，两后端）、P2-17（Buffer 边界溢出）、P2-19（D3D12 描述符视图越界）。
3. **P0-1 → P0-2/3/4**：这条链串起来就能拿到 Vulkan 的第一张三角形。建议同一批做，单独修任何一个都看不到图。
4. **P1-2 → P1-11 → P1-12 → P1-1**：先修「绑错 set」「空纹理槽位」「descriptor type 不匹配」这三个硬错误，再决定 root descriptor 的长期方案（推荐先立「Vulkan 只支持 descriptor table + push constant」的约束）。
5. **P1-10 / P1-18 / P1-19**：push-constant 索引、深度 aspect、反射分类，三个都是「绑定看起来成功但值不对」的隐蔽错误，做多参数/多资源场景前必须修。
6. **P1-3 / P1-4 / P1-5 / P1-6 / P1-7 / P1-8 / P1-13 / P1-14 / P1-15 / P1-16 / P1-17**：状态类与描述符类，做 3D 场景前必须补齐；D3D12 的三个 `Convert*` 优先级高于 Vulkan 的同名空函数（因为 D3D12 是「主路径」）。
7. **P2-1 / P2-2 / P2-3 / P2-6 / P2-15 / P2-16**：同步正确性与并发。P2-6 属正确性，提前到第 4 步；P2-15/P2-16 是 JobSystem 的死锁与空转，影响 `SubsystemControl` 退出，建议和第 2 步一起处理。
8. **P2-9 / P2-10 / P2-11 / P2-12 / P2-13 / P2-14 / P2-19**：生命周期与泄漏，稳定出图后逐个修，且必须配合校验层验证。
9. **P2-4 / P2-5 / P2-7 / P2-8 / P3-5**：性能、交换链重建与测试收尾，最后做。

---

## 4. 修复后的验收清单

- [ ] `RHI_D3D12.dll`、`RHI_Vulkan.dll`、`HelloTriangleTest.exe` 均生成
- [ ] `HelloTriangleTest` 在 D3D12 下出三角形（基线）
- [ ] `HelloTriangleTest` 在 Vulkan 下出**同样的**三角形（P0 全部通过）
- [ ] Vulkan 校验层（`VK_LAYER_KHRONOS_validation`）在整帧循环里零 error / 零 warning
- [ ] ASan（`/fsanitize=address`）下创建 1 个 256 对齐的 CB，无越界读告警（P1-9）
- [ ] 一个 PSO 带 2 个 Constants 参数（同 stageFlags 触发合并），两个常量都读到正确值（P1-10）
- [ ] 纹理 + 采样器用例：两后端采样结果一致，且 VK 侧校验层无 descriptor 相关 error（P1-11/12/13）
- [ ] 带偏移地绑定顶点缓冲（`pOffsets != 0`）两后端一致（P1-15）
- [ ] 改窗口尺寸连续 20 次不崩、不出 `OUT_OF_DATE` 死循环（P2-7）
- [ ] 「关背面剔除 + 开 alpha blend + 开深度测试」用例两后端出图一致（P1-4/6/7/8/16/17）
- [ ] 一张 PSO 带两个 descriptor table（第二个带非零偏移）用例两后端一致（P1-2）
- [ ] 创建 100 个 buffer 的耗时：只发生 1 次 GPU 往返（P2-4）
- [ ] `JobSystem::Uninstall` 后队列已排空，无 `Wait` 空转（P2-15）；`ScheduleBatch` 内工厂再 `Schedule` 不死锁（P2-16）
- [ ] `TestSwapChain` 手动关窗后 1 秒内退出，不跑满 5 秒（P3-5）

---

## 5. 审查范围与假设

**已逐行读**：
- `RHI`：`RHIInterface.h`、`IRHIModule.h`、`RHICommandList.h`、`RHIResource.h`、`RHIRootSignature.h`、`RHIPipelineState.h`、`Common/RHIDefinitions.h`、`Common/RHIConfig.h`、`RHILoader.cpp`、`RHIPlatformDetection.cpp`、`RHIShaderCompiler/*`（CompilePipeline / CompilerInstance / SPIRV 反射入口）
- `D3D12RHI`：`RHID3D12.cpp`、`RHICommandListD3D12.cpp`、`SwapChainD3D12.cpp`、`RHIResourceD3D12.cpp`、`RHIRootSignatureD3D12.cpp`、`RHIPipelineStateD3D12.cpp`、`CreateShaderD3D12.cpp`、`ShaderCompilerBackendD3D12.cpp`、`CommandListDX12_*`（全部 9 个）
- `VulKanRHI`：`RHIVulKan.cpp/.h`、`RHICommandListVulKan.cpp/.h`、`SwapChainVulkan.cpp`、`RHIResourceVulKan.cpp`、`RHIRootSignatureVulKan.cpp/.h`、`RHIPipelineStateVulkan.cpp`、`CreateShaderVulKan.cpp`、`ShaderCompilerBackendVulkan.cpp`、`DescriptorHeapVulKan.cpp/.h`、`DescriptorViewVulKan.cpp`、`CommandListVK_*`（全部 9 个）
- `Core`（补读）：`Private/Jobsystem/JobSystem.cpp`、`Public/Tools/Buffer.h`（`BinaryWrite` / `BinaryReader`）
- `RHI`（补读）：`RHIShaderCompiler/RHISPIRVGenerationReflection.cpp`
- `Tests`：`HelloTriangleTest.cpp`、`TestSwapChain.cpp`（+ 两个测试的目录结构）

**未逐行展开**（结论未引用其内部细节）：`Core` 模块其余部分（日志捕获/断言宏/`SubsystemControl` 实现）、`D3D12RHI/DescriptorViewD3D12.cpp` 的 buffer 分支细节、`SPIRV-Cross` 与 `DirectXShaderCompiler` 第三方、`SingleHeaderInit*` 的初始化聚合、`ThirdParty` 全部。

**假设**：
- `ThirdParty/VulkanSdk` 是完整 SDK（目录存在，含 `Include`/`lib`/`dll`），未校验版本。
- 未做运行时验证，所有「能画出/画不出」的结论来自代码路径完整性，不是实测。
- 附录 A 的旧文档条目判定标准是「回源码逐条核对」，只判断缺陷是否成立，不评价旧文档的分级与措辞。

---

## 附录 A：旧缺陷文档《代码缺陷.md》28 条核对结果

判定口径：**属实且新增** = 本文此前未覆盖、回源码确认成立；**已覆盖** = 与本文某条为同一缺陷；**需更正** = 旧文档描述与实际代码不符。

| 旧编号 | 旧文档要点 | 判定 | 对应本文 |
|---|---|---|---|
| 1 | 任务系统卸载时会遗弃排队任务 | 属实且新增（「永久阻塞」应表述为 `Wait` 空转） | P2-15 |
| 2 | Vulkan 图形管线创建始终失败 | 已覆盖（行号应为 `:158`，旧文档写 `:156`） | P0-1 |
| 3 | Vulkan 多个图形命令接口为空操作 | **描述有误**：`ClearRenderTargetView` 与 `OMSetRenderTargets` **都已实现**；真正空的是 `ClearDepthStencilView:36`、`OMSetBlendState:113`、`OMSetDepthStencilState:118` | P1-4 / P1-5 |
| 4 | Vulkan 纹理/采样器描述符没有创建实际资源视图 | 属实且新增（采样器 `nullptr` 部分已覆盖） | P1-11（新增） + P1-3 |
| 5 | Vulkan 描述符表绑定忽略调用方指定的表位置 | 已覆盖（旧文档引 `:71`，本文引消费点 `:94,198`） | P1-2 |
| 6 | Vulkan push-constant 合并后仍用旧索引 | 属实且新增 | P1-10 |
| 7 | Vulkan 常量缓冲区初始化越界读调用方数据 | 属实且新增，**且 D3D12 侧同源**（旧文档只提 Vulkan） | P1-9 |
| 8 | Vulkan Buffer SRV 描述符类型与用途不匹配 | 属实且新增 | P1-12 |
| 9 | Vulkan 关闭先销毁描述符池后 `vkDeviceWaitIdle` | 属实且新增 | P2-10 |
| 10 | 释放槽位但不释放 descriptor set，最终耗尽池 | 属实且新增 | P2-9 |
| 11 | D3D12 关闭在 GPU 完成前销毁描述符堆 | 属实且新增 | P2-11 |
| 12 | D3D12 纹理描述符返回错误类型的句柄 | 属实且新增 | P1-13 |
| 13 | D3D12 `CopyResource` 错误转换为 Buffer | 属实且新增 | P1-14 |
| 14 | 批量任务工厂持锁时调用用户代码 | 属实且新增 | P2-16 |
| 15 | `BinaryReader` 边界检查整数溢出 | 属实且新增（与 16 合并为一条） | P2-17 |
| 16 | `BinaryWrite` 容量计算溢出 | 属实且新增（与 15 合并为一条） | P2-17 |
| 17 | `MoveData` 移出数据后未重置元数据 | 属实且新增 | P2-18 |
| 18 | D3D12 深度/模板清除与输出合并状态为空操作 | 属实且新增（OM 部分已覆盖） | P1-16（新增） + P1-8 |
| 19 | D3D12 顶点缓冲绑定忽略输入偏移 | 属实且新增 | P1-15 |
| 20 | D3D12 描述符视图缺少索引边界检查 | 属实且新增 | P2-19 |
| 21 | D3D12 根签名描述符范围数组泄漏 | 属实且新增 | P2-12 |
| 22 | D3D12 `Unmap` 报告写入范围为空 | 属实且新增（后果需按堆类型区分，见条目说明） | P2-13 |
| 23 | D3D12 CB 对齐只检查完全相等的标志 | 属实且新增 | P1-17 |
| 24 | Vulkan Readback 映射未处理非一致性内存失效 | 属实且新增 | P2-14 |
| 25 | Vulkan 深度屏障为无模板格式带 stencil aspect | 属实且新增 | P1-18 |
| 26 | SPIR-V 反射把 push constant 当普通 descriptor | 属实且新增 | P1-19 |
| 27 | 编译器 Begin/End 在 Release 下空指针 | 属实且新增 | P1-20 |
| 28 | 交换链测试关窗后仍继续提交渲染 | 属实且新增 | P3-5 |

> 合计：旧文档 28 条中 **25 条属实且为本文遗漏项**（第 15、16 条合并为 P2-17），**2 条与本文重复**（第 2、5 条），**1 条描述错误**（第 3 条）。本文条目总数由 24 条增至 **48 条**（新增 P1-9~P1-20、P2-9~P2-19、P3-5）。