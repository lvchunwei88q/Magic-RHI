# MagicRHI 问题清单（D3D12 / Vulkan 双后端）

> 首次审查：2026-11-04；**本次复核：2026-11-05，仓库 HEAD `18d30c2`**（`18d30c267f3876c6c47c9cce5c40ff87ec5cb021`）。
> 复核方式：**静态代码走查**，逐条回当前 HEAD 源码重新定位行号；**本次复核未运行任何可执行文件，也未启用校验层**（校验层 DLL 在 `Binary\Debug` 可用，见下文「构建现状」，但本次复核未加载）。凡涉及 barrier 合法性、内存一致性、GPU 侧可见性的结论，均标注「代码已补，运行时未验证」。
> 版本说明：本文已合并外部旧缺陷文档《代码缺陷.md》的 28 条（判定见附录 A），本次复核保留原 48 个编号（P0×4 / P1×20 / P2×19 / P3×5），**未重排、未合并、未删除、未插入新编号**；本次复核新发现的问题单列在文末「新增（未编号，复核于 2026-11-05）」。
> 构建现状：存在两套产物。`out\bin\` 是 2026-05-27 ~ 2026-06-01 的旧 D3D11/D3D12 构建（`Magic-DirectX11RHI.dll`、`Magic-DirectX12RHI.dll`、`Magic-RHI.dll`、`Magic-Core.dll`、`Magic-IO.dll`、`MagicRHI.exe`），不含 Vulkan 后端。当前实际产物在 `Binary\Debug\`，整包构建于 **2026-11-05 09:39**，含 `Magic-VulKanRHI.dll`（1,393,664 B）、`Magic-RHI.dll`（5,108,736 B）、`Magic-Core.dll`（1,090,560 B）、`Magic-D3D12RHI.dll`（1,214,976 B），以及 `HelloTriangleTest.exe`（235,008 B）、`TestSwapChain.exe`（130,560 B）、`TestShaderCompile.exe`（249,856 B）；同目录另有校验层与转译器 `VkLayer_khronos_validation.dll`、`VkLayer_api_dump.dll`、`VkLayer_crash_diagnostic.dll`（2026-10-29）、`dxcompiler.dll`。该包时间戳（09:39）比 HEAD `18d30c2`（2026-11-05 10:24:45）早约 45 分钟；HEAD 那条提交只改构建脚本里的 VS Dev CMD 路径、不动运行时代码，故这批产物基本对应当前代码状态。**本次复核未运行这些产物、未加载校验层**；下文所有结论仍以代码层证据为准，出图/合法性判断属运行时未验证。
> 引用格式：`文件:行`，路径相对于仓库根 `F:\ModernDevelopment\DirectXRHI`。

---

## 0. 一句话结论

接口按 D3D12 语义建模：**D3D12 后端是透传方，Vulkan 后端是翻译方**。D3D12 每帧链路基本可跑（状态类参数取默认值）；Vulkan 图形链路相较 2026-11-04 已有两处打通——**管线绑定**（`vkCmdBindPipeline` 已按图形/计算两个 bind point 发出，并记录当前 PSO）与**视口/裁剪矩形**（`vkCmdSetViewport` / `vkCmdSetScissor` 已实现，含 D3D→Vulkan 的 Y 轴翻转）。

但当前 VK 出图仍卡在两个断点上：

1. **图形 PSO 从未创建**：`RHIPipelineStateVulkan` 图形分支填完 shader stage 后直接 `return false`，从未调用 `vkCreateGraphicsPipelines`，上层拿到空 PSO，`IsValid()` 为 false；
2. **输入装配（IA）仍为空**：`IASetPrimitiveTopology` / `IASetVertexBuffers` / `IASetIndexBuffer` 三个函数体全空，顶点/索引缓冲不绑定。

即使这两处补齐，链路后面仍有一批未通的环节：纹理/采样器描述符只占槽不建 view、root-level CBV/SRV/UAV 未实现、descriptor set 绑错堆与偏移、Buffer SRV 的 descriptor type 与 usage 不匹配等（详见第 1、2 节）。底层生命周期/并发/性能问题（描述符池回收、关闭顺序、JobSystem 队列遗弃、Buffer 边界等）在「能出图」之后仍会继续咬人。

---

## 1. 总览

共 **48 条**。状态口径：**已修复**（静态可证，附代码位置）/ **部分修复**（写清缺哪一半）/ **未修复** / **描述有误**。

**修复进度统计（复核于 2026-11-05，HEAD 18d30c2）**：已修复 **2**（P0-2、P0-4）；部分修复 **1**（P2-7）；未修复 **45**；描述有误 **0**。
分级剩余：**P0 未修复 2**（P0-1、P0-3）/ 已修复 2；**P1 未修复 20**；**P2 未修复 18** / 部分修复 1；**P3 未修复 5**。

| 编号 | 级别 | 状态 | 位置 | 问题 | 后果 |
|---|---|---|---|---|---|
| P0-1 | 阻塞 | 未修复 | `VulKanRHI/Private/RHIPipelineStateVulkan.cpp:156-158` | 图形 PSO 填完 shader stage 后直接 `return false`，从未调用 `vkCreateGraphicsPipelines` | 上层拿到 `nullptr`，PSO `IsValid()` 为 false，HelloTriangle 断言必挂 |
| P0-2 | 阻塞 | 已修复 | `VulKanRHI/Private/CommandList/CommandListVK_PipelineState.cpp:29-38` | 已按图形/计算 bind point 调 `vkCmdBindPipeline`，并记录 `m_pCurrentGraphicsPSO` / `m_pCurrentComputePSO` | 管线绑定命令已进入命令流；仍受 P0-1 无有效 PSO 牵连 |
| P0-3 | 阻塞 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_InputAssembly.cpp:10-20` | `IASetPrimitiveTopology` / `IASetVertexBuffers` / `IASetIndexBuffer` 全空函数体 | 无顶点/索引缓冲绑定，无拓扑设置 |
| P0-4 | 阻塞 | 已修复 | `VulKanRHI/Private/CommandList/CommandListVK_Rasterizer.cpp:5-37` | `RSSetViewports` / `RSSetScissorRects` 已实现（含 Y 轴翻转：`y=topLeftY+height`、`height` 取负） | 视口/裁剪命令已进入命令流 |
| P1-1 | 功能缺失 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_RootSignature.cpp:107-132,211-236` | 六个 root CBV/SRV/UAV 绑定全 `// TODO` | 只有 descriptor table + push constant 生效 |
| P1-2 | 逻辑错误 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_RootSignature.cpp:89-95,193-199` | 绑 descriptor set 时把 heap 写死成 `{Standard,0}`，且忽略 `offsetInDescriptorsFromTableStart` | 多表/带偏移时绑到同一份错误 set |
| P1-3 | 功能缺失 | 未修复 | `VulKanRHI/Private/RHIResourceVulKan.cpp:110-117` | `CreateSamplerState` 直接 `return nullptr`，`DeleteSamplerState` 空 | 采样器永不可用，写 descriptor 必带空指针 |
| P1-4 | 功能缺失 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_OutputMerger.cpp:113-121` | `OMSetBlendState` / `OMSetDepthStencilState` 空 | VK 侧混合/深度状态不可配 |
| P1-5 | 功能缺失 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_Clear.cpp:36-38` | `ClearDepthStencilView` 空函数体 | 深度/模板无法 clear |
| P1-6 | 功能缺失 | 未修复 | `D3D12RHI/Private/RHIPipelineStateD3D12.cpp:25-53`，调用点 `:146,148,149` | `ConvertBlendState` / `ConvertRasterizerState` / `ConvertDepthStencilState` 三个函数体都是 `// TODO` | PSO 里 blend/raster/depth 全为默认值 |
| P1-7 | 逻辑错误 | 未修复 | `D3D12RHI/Private/RHIPipelineStateD3D12.cpp:162` | `PrimitiveTopologyType` 写死 `D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE` | 点/线/细分拓扑无法用 |
| P1-8 | 功能缺失 | 未修复 | `D3D12RHI/Private/CommandList/CommandListDX12_OutputMerger.cpp:32-40` | `OMSetBlendState` / `OMSetDepthStencilState` 空 | D3D12 侧同样不可动态配状态 |
| P1-9 | 内存安全 | 未修复 | `VulKanRHI/Private/RHIResourceVulKan.cpp:126,144`、`D3D12RHI/Private/RHIResourceD3D12.cpp:177,186` | CB 尺寸按 256 对齐后，`memcpy` 仍用对齐后大小拷贝 `InitialData` | 调用方只提供原尺寸缓冲区 → **越界读**（两个后端都有） |
| P1-10 | 逻辑错误 | 未修复 | `VulKanRHI/Private/RHIRootSignatureVulKan.cpp:324-329,342`、`CommandListVK_RootSignature.cpp:149,253` | push-constant range 索引在合并前保存、合并后使用 | 多个 Constants 参数时偏移/范围错位，常量互相覆盖 |
| P1-11 | 功能缺失 | 未修复 | `VulKanRHI/Private/DescriptorViewVulKan.cpp:90-108` | 纹理/采样器描述符入口只 `Allocate()` 一个槽位，不建 `VkImageView`/`VkSampler`、不写 descriptor | 句柄「看起来有效」但无法绑到 GPU |
| P1-12 | 逻辑错误 | 未修复 | `VulKanRHI/Private/RHIResourceVulKan.cpp:37-38`、`DescriptorHeapVulKan.cpp:275` | Buffer SRV 写成 `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER`，但创建时只声明了 `STORAGE_TEXEL_BUFFER` / `UNIFORM_TEXEL_BUFFER` | 违反 usage/descriptor 匹配，校验层报错、采样读不到 |
| P1-13 | 功能缺失 | 未修复 | `D3D12RHI/Private/DescriptorViewD3D12.cpp:91-109` | 纹理描述符从 **sampler 堆** 分配，`Type` 被忽略，不创建 SRV/UAV；`CreateSamplerHeapDescriptorView` 也只占槽不建 sampler | 句柄属于错误的堆类型，必然绑不上 |
| P1-14 | 正确性 | 未修复 | `D3D12RHI/Private/CommandList/CommandListDX12_Resource.cpp:9-10` | `CopyResource` 对 `RHIResource*` 做 `static_cast<BufferD3D12*>`，无类型校验 | 传纹理进去即 UB，访问错误的资源指针 |
| P1-15 | 逻辑错误 | 未修复 | `D3D12RHI/Private/CommandList/CommandListDX12_InputAssembly.cpp:97,99` | `IASetVertexBuffers` 完全忽略 `pOffsets`，`SizeInBytes` 也用整缓冲大小 | 非零偏移时读到错误顶点数据 |
| P1-16 | 功能缺失 | 未修复 | `D3D12RHI/Private/CommandList/CommandListDX12_Clear.cpp:14-17` | `ClearDepthStencilView` 空函数体（只有 `// TODO`） | 深度/模板无法 clear（VK 侧同样空，见 P1-5） |
| P1-17 | 逻辑错误 | 未修复 | `D3D12RHI/Private/RHIResourceD3D12.cpp:173-175` | 对齐判断用 `flag == BufferBindFlag::ConstantBuffer`（相等），组合标志会漏 | CB + 其他标志组合时不对齐，CBV 无效；VK 侧用的是 `EnumHasAnyFlags`，是对的 |
| P1-18 | 正确性 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_ResourceBarrier.cpp:95-103` | `GetAspectMask` 只要涉及深度就固定返回 `DEPTH_BIT|STENCIL_BIT` | D16/D32 等无模板格式的 barrier 非法 |
| P1-19 | 正确性 | 未修复 | `RHI/Private/RHIShaderCompiler/RHISPIRVGenerationReflection.cpp:156-172` | push constant 被当作普通 descriptor 读 `DescriptorSet`/`Binding` decoration | 缺省拿到 0/0，混进 `resourcesBySet[0]` 与真实资源冲突 |
| P1-20 | 崩溃风险 | 未修复 | `RHI/Private/RHIShaderCompiler/RHICompilePipeline.cpp:50-56,66-72` | `controller` 判空只在 `#if RHI_ENABLE_DEBUG_INFO` 内，Release 下继续解引用 | Release 构建空指针崩溃 |
| P2-1 | 性能/语义 | 未修复 | `VulKanRHI/Internal/RHICommandListVulKan.h:224`、`RHICommandListVulKan.cpp:187,230,241` | 只有 1 个 `m_inFlightFence`，3 个后备缓冲被串行化 | 实际只有 1 帧在飞，CPU/GPU 无法重叠 |
| P2-2 | 资源管理 | 未修复 | `VulKanRHI/Private/RHICommandListVulKan.cpp:191-204,225` | 信号量按 `imageIndex` 建，fence 只有 1 个；resize 时旧信号量不销毁 | 同步模型自相矛盾；窗口来回改尺寸会漏信号量 |
| P2-3 | 语义隐患 | 未修复 | `D3D12RHI/Private/RHICommandListD3D12.cpp:229,240,251` | 提交时写 `m_FrameFenceValues[m_CurrentFrameIndex]`，但槽号要到 `EndFrame` 才更新 | 记进数组的其实是上一帧槽位，能跑但语义错 |
| P2-4 | 性能 | 未修复 | `D3D12RHI/Private/RHIResourceD3D12.cpp:261`、`VulKanRHI/Private/RHIResourceVulKan.cpp:202` | `CreateBuffer` 内部为了上传初始数据直接 `WaitForGPU` | 加载期完全同步，无异步上传 |
| P2-5 | 性能/扩展性 | 未修复 | `VulKanRHI/Private/RHIResourceVulKan.cpp:94` | 每个 buffer 一次 `vkAllocateMemory` | 无 sub-allocation，资源数量受 `memoryTypeCount` 限制 |
| P2-6 | 逻辑错误 | 未修复 | `VulKanRHI/Private/CommandList/CommandListVK_OutputMerger.cpp:104-111` | `EndDynamicRendering` 不清 `m_CurrentRenderTargets` | 一帧内二次 `OMSetRenderTargets` 后，`ClearRenderTargetView` 的附件索引指向旧 RTV |
| P2-7 | 健壮性 | 部分修复 | `VulKanRHI/Private/SwapChainVulkan.cpp:147-158,169-192`、`D3D12RHI/Private/SwapChainD3D12.cpp:153-174`；Present 侧 `RHICommandListVulKan.cpp:270-273`、`RHICommandListD3D12.cpp:262-273` | 两侧都补了 `Resize` 重建路径（VK 在 `AcquireNextImage` 遇 OUT_OF_DATE 时触发；D3D12 由测试 WM_SIZE 触发）；但 **Present 侧仍无 OUT_OF_DATE / SUBOPTIMAL / 遮挡的重建处理**，VK 只打警告，D3D12 只判 DEVICE_REMOVED/RESET |  Present 返回过期/遮挡时仍不会自动重建；尺寸以外的路径仍可能持续异常 |
| P2-8 | 性能 | 未修复 | `VulKanRHI/Private/RHICommandListVulKan.cpp:95-103`、`Internal/RHICommandListVulKan.h:62` | 每次 `CreateCommandList` 都 `vkAllocateCommandBuffers(1)`，析构 `= default` 不释放 | 每帧新建 command buffer，长期碎片化 |
| P2-9 | 资源泄漏 | 未修复 | `VulKanRHI/Private/DescriptorHeapVulKan.cpp:109-117` | `Free` 只清本地记录，全程没有 `vkFreeDescriptorSets` / `vkResetDescriptorPool` | 反复分配/释放持续消耗池，最终耗尽 |
| P2-10 | 生命周期 | 未修复 | `VulKanRHI/Private/RHIVulKan.cpp:886-893,905` | `Shutdown` 先 reset 4 个描述符堆与队列，之后才 `vkDeviceWaitIdle` | 已提交命令仍引用这些 set 时销毁，违反 Vulkan 生命周期 |
| P2-11 | 生命周期 | 未修复 | `D3D12RHI/Private/RHID3D12.cpp:278-285`；队列析构 `RHICommandListD3D12.cpp:203` | `Shutdown` 先 reset 4 个堆，队列析构里才 `WaitForGPU` | 堆先于 GPU 完成销毁，属同步责任错位 |
| P2-12 | 内存泄漏 | 未修复 | `D3D12RHI/Private/RHIRootSignatureD3D12.cpp:45` | `TranslateRanges` 里 `new[]`，序列化后无 `delete[]` | 每建一次根签名泄漏 range 数组 |
| P2-13 | 语义错误 | 未修复 | `D3D12RHI/Internal/RHIResourceD3D12.h:85-86` | `Unmap` 传 `D3D12_RANGE writeRange(0, 0)`，即「零字节被写」 | 驱动不感知 CPU 写入；将来用于 readback 会读到旧数据 |
| P2-14 | 正确性 | 未修复 | `VulKanRHI/Private/RHIResourceVulKan.cpp:19-20` | `Readback` 只要求 `HOST_VISIBLE|HOST_CACHED`，允许 non-coherent | GPU 写入后 CPU 读无 `vkInvalidateMappedMemoryRanges`，读到旧数据 |
| P2-15 | 正确性 | 未修复 | `Core/Private/Jobsystem/JobSystem.cpp:69-80,130-136,162-170` | `Uninstall` 置 `running=false` 后 join，队列剩余任务不再执行；`WorkerLoop` 直接退出 | 等待这些任务句柄的 `Wait` 永久空转（`yield` 死循环） |
| P2-16 | 死锁 | 未修复 | `Core/Private/Jobsystem/JobSystem.cpp:102-106` | `ScheduleBatch` 持 `queueMutex` 时调用用户工厂 `factory(i)` | 工厂内再 `Schedule` 即重入非递归锁 → 死锁 |
| P2-17 | 内存安全 | 未修复 | `Core/Public/Tools/Buffer.h:54,178` | `m_WriteOffset + size`、`m_ReadOffset + count` 先加后比，无溢出检查 | 加法溢出可绕过容量检查，后续 memcpy 越界 |
| P2-18 | 状态不一致 | 未修复 | `Core/Public/Tools/Buffer.h:108-111` | `MoveData` 只 `ShrinkToFit()` + 移走 vector，不重置 `m_WriteOffset`/`m_CurrentSize` | 移出后继续写会写到已空缓冲区的旧偏移 |
| P2-19 | 越界访问 | 未修复 | `D3D12RHI/Private/DescriptorHeapD3D12.cpp:53-54` | `GetDescriptorHeapView` 校验了句柄类型但没校验 `index < Capacity` | 越界索引访问 `m_Descriptors[index]`（VK 侧 `DescriptorHeapVulKan.cpp:94` 有该检查） |
| P3-1 | 构建隐患 | 未修复 | `VulKanRHI/Public/RHIVulKan.h:9`；`Internal/RHICommandListVulKan.h:4`、`Internal/RHIResourceVulKan.h:3`、`Internal/DescriptorHeapVulKan.h:3`、`Internal/RHIRootSignatureVulKan.h:5`、`Public/RHIFormatVulkan.h:4` | `VK_USE_PLATFORM_WIN32_KHR` 用 `#define` 写在头里，而多个头先直接 include `<vulkan.h>` | 包含顺序敏感，换 TU 顺序就可能编不过 `vkCreateWin32SurfaceKHR` |
| P3-2 | 可维护性 | 未修复 | `CommandListVK_*` 全体（例 `CommandListVK_RootSignature.cpp:73,77,141`） | 失败路径混用「静默 `return`」和 `ThrowErrorMessage` | 半成品不报错，只能靠校验层兜底 |
| P3-3 | 代码卫生 | 未修复 | `VulKanRHI/Private/RHIPipelineStateVulkan.cpp:62-154,158` | graphics 分支构造的 `VkPipelineShaderStageCreateInfo` 全部未被使用即 `return false` | 死代码，易误判「已实现」 |
| P3-4 | 测试 | 未修复 | `Tests/HelloTriangleTest/HelloTriangleTest.cpp:252-253` | 断言依赖 `graphicsPSO->IsValid()`，未按后端分支跳过 | 选 Vulkan 后端必挂，且失败点远离真因 |
| P3-5 | 测试 | 未修复 | `Tests/TestSwapChain/TestSwapChain.cpp:17,86,96-100` | `WM_DESTROY` 里直接 `return 0`，不发退出消息；循环 `while(true)` 只靠 5 秒超时退出 | 用户关窗后仍向已销毁窗口的交换链提交，直到超时 |

---

## 2. 详细条目

### P0-1 图形 PSO 从未创建

**位置**：`Engine/Source/Runtime/VulKanRHI/Private/RHIPipelineStateVulkan.cpp:156-158`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `RHIPipelineStateVulkan.cpp:158`（`return false;`）；计算管线对照 `:224`（`vkCreateComputePipelines`）**

**现象**：`Initialize(Device*, const GraphicsPipelineStateDesc&)` 从 `:20` 开始，逐段校验 device / rootSignature / VS / PS / GS / HS / DS 并填好 `VkPipelineShaderStageCreateInfo`（`shaderStages` 构造于 `:62-154`），最后在 `:156-157` 只设置 `Type` 与 `m_Device`，`:158` 直接：

```cpp
        Type = PipelineStateType::Graphics;
        m_Device = VKDevice;
        return false;   // :158 —— 前面全是校验与 stage 收集，从来没有 vkCreateGraphicsPipelines
```

对比计算管线 `:216-239` 是真的调了 `vkCreateComputePipelines` 并写入 `m_Pipeline`，所以 **Vulkan 侧只有 compute 能用**。

**影响**：`DeviceVulKan::CreateGraphicsPipelineState`（`:265-273`）拿不到有效的 `VkPipeline`，返回 `nullptr`；即便构造出对象，`IsValid()`（`:254-256`）也因 `m_Pipeline == VK_NULL_HANDLE` 为 false。这是当前 VK 图形链的首要根断点。

**建议修法**（旧建议保留，尚未实现）：
1. 按 `desc` 组 `VkGraphicsPipelineCreateInfo`：
   - `pVertexInputState` 由 `desc.pInputLayout` 转 `VkVertexInputBindingDescription` + `VkVertexInputAttributeDescription`；
   - `pInputAssemblyState` 的 `topology` 与 `pRasterizationState` 的 `polygonMode/cullMode/frontFace` 由 `desc` 转；
   - `pViewportState` 里 `viewportCount/scissorCount=1`，并在 `VkPipelineDynamicStateCreateInfo` 声明 `VK_DYNAMIC_STATE_VIEWPORT`、`VK_DYNAMIC_STATE_SCISSOR`（对应已实现的 P0-4 `vkCmdSet*`，否则 dynamic state 不生效）；
   - `pColorBlendState` 按 `desc.PipelineRenderTargetBlendDesc` 生成 `VkPipelineColorBlendAttachmentState`（可先只支持一档简单的 alpha blend，其余留 warning）；
   - `pDepthStencilState` 按 `desc` 生成，enabled 由是否带 DSV 决定；
   - `layout = GetPipelineLayout()`；
   - **关键**：dynamic rendering 必须在 `VkPipelineRenderingCreateInfoKHR`（`pNext`）里给出 `colorAttachmentFormats[]` 与 `depthFormat`，否则校验层直接报错、管线不可用。
2. `vkCreateGraphicsPipelines(...)`，失败时 `ThrowErrorMessage` + `return false`。
3. 顺手把 `:62-154` 那批已收集的 `shaderStages` 真正接进 `pStages`，消除 P3-3。

**验证**：HelloTriangle 用 `RHIType::VulKan` 跑通到「PSO 非空且 `IsValid()`」；Vulkan 校验层无 error（运行时未验证）。

---

### P0-2 / P0-3 / P0-4 录制层管线绑定、IA、视口裁剪

#### P0-2 管线绑定 —— **已修复**

**位置**：`Engine/Source/Runtime/VulKanRHI/Private/CommandList/CommandListVK_PipelineState.cpp:29-38`

**当前状态：已修复 | 复核 2026-11-05 | 证据 `CommandListVK_PipelineState.cpp:31`（图形 `vkCmdBindPipeline`）、`:32`（`m_pCurrentGraphicsPSO`）、`:36`（计算 `vkCmdBindPipeline`）、`:37`（`m_pCurrentComputePSO`）**

`SetPipelineState` 已按 `stateType` 分流：图形走 `VK_PIPELINE_BIND_POINT_GRAPHICS`，计算走 `VK_PIPELINE_BIND_POINT_COMPUTE`，并把 PSO 缓存到 `m_pCurrentGraphicsPSO` / `m_pCurrentComputePSO`（成员声明 `Internal/RHICommandListVulKan.h:150-151`）。修复与旧建议一致；旧建议里「配合 dynamic state 立刻补一次 viewport/scissor」未做——视口/裁剪由上层 `RSSet*` 命令承担（P0-4 已实现）。

> 牵连说明：`vkCmdBindPipeline` 传入的 `pso->GetPipeline()` 在 P0-1 未修前仍是 `VK_NULL_HANDLE`，本条目只是命令路径打通，端到端可用性仍受 P0-1 阻断。**代码已补，运行时未验证。**

#### P0-3 输入装配 —— **未修复**

**位置**：`Engine/Source/Runtime/VulKanRHI/Private/CommandList/CommandListVK_InputAssembly.cpp:10-20`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:10-12`（`IASetPrimitiveTopology` 空体）、`:14-16`（`IASetVertexBuffers` 空体）、`:18-20`（`IASetIndexBuffer` 空体）**

三个函数仍是空函数体，没有任何 `vkCmdBindVertexBuffers` / `vkCmdBindIndexBuffer` / 拓扑设置。

**影响**：即便 P0-1 造出有效 PSO，命令流里仍没有顶点/索引缓冲绑定与拓扑，绘制不会产生正确图元。

**建议修法**（旧建议保留）：
```cpp
// IASetVertexBuffers
std::vector<VkBuffer> bufs(n); std::vector<VkDeviceSize> offs(n);
// BufferVulKan::GetBuffer() / GetSize()
vkCmdBindVertexBuffers(m_CommandBuffer, startSlot, numBuffers, bufs.data(), offs.data());

// IASetIndexBuffer
vkCmdBindIndexBuffer(m_CommandBuffer, SafeCast<BufferVulKan>(pIndexBuffer)->GetBuffer(), offset,
                     format == RHIIndexFormat::UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
```
拓扑需转成 `VkPrimitiveTopology`，并在 PSO 创建时与 IA state 对齐（见 P0-1 修法）。

#### P0-4 视口 / 裁剪矩形 —— **已修复**

**位置**：`Engine/Source/Runtime/VulKanRHI/Private/CommandList/CommandListVK_Rasterizer.cpp:5-37`

**当前状态：已修复 | 复核 2026-11-05 | 证据 `:20`（`vkCmdSetViewport`）、`:36`（`vkCmdSetScissor`）；Y 轴翻转 `:14`（`y = topLeftY + height`）、`:16`（`height = -height`）**

`RSSetViewports` / `RSSetScissorRects` 均已实现：视口按 RHIViewport 转换并做 D3D→Vulkan Y 轴翻转；裁剪矩形按 RHIRect 的 left/top/right/bottom 转 `VkRect2D`（`w/h <= 0` 时截为 0）。

> 实现与旧建议的差异：旧建议未提 Y 翻转；实际实现把 `y` 下移一个高度并把 `height` 取负（`:14,:16`），这是 D3D NDC 向上、Vulkan NDC 向下的等价翻转，属于正确的实现细节。**代码已补，运行时未验证。**

**验证（P0-2/P0-3/P0-4 共同）**：Vulkan 校验层开启后跑 HelloTriangle，不应再出现 `VUID-vkCmdDraw-None-*`（未绑管线/未绑顶点缓冲）类报错（运行时未验证）。

---

### P1-1 Vulkan 无 root-level CBV/SRV/UAV

**位置**：`CommandListVK_RootSignature.cpp:107-114`、`:116-123`、`:125-132`（graphics）、`:211-218`、`:220-227`、`:229-236`（compute）

**当前状态：未修复 | 复核 2026-11-05 | 证据 六个函数体均为 `// TODO` + `(void)` 消除未使用参数，如 `:112-113,121-122,130-131,216-217,225-226,234-235`**

**现象**：六个函数体全是 `// TODO`；debug 模式靠 `CHECK_BUFFER_TYPE` 宏打一条 warning。文件顶部 `:30-36` 的注释仍说明这是接口与目标 API 的结构性不匹配，不是漏写。

**为什么值得单独列**：D3D12 允许 root signature 里直接放一个 GPU 虚拟地址的 CBV/SRV/UAV，Vulkan 没有这个概念。另见新增 N3：`BufferVulKan::GetGPUVirtualAddress` 恒返回 0，VK 侧连地址都取不到。

**建议修法**（三选一，按改动量排序）：
1. **每 root 参数一个极小 descriptor set**：在 `RHIRootSignatureVulKan` 里为每个 `RootDescriptor` 类型参数建一个 1-binding set layout，绑定时 `vkUpdateDescriptorSets` 写一条再 `vkCmdBindDescriptorSets`。
2. **`VK_KHR_push_descriptors`**：直接 `vkCmdPushDescriptorSetKHR`，每帧零分配；需查扩展支持并在 device 创建时启用，同时改 `VkPipelineLayoutCreateInfo` 的 `flags |= VK_PIPELINE_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR`。
3. **统一到 descriptor heap/table**：把 root descriptor 在 RHI 层就降级成 table + offset，Vulkan 侧不需要新原语，但会改变上层用法。

> 建议：短期内先在 RHI 层立一条约束「Vulkan 后端只保证 descriptor table + 32bit constants」并在文档里写死。

---

### P1-2 descriptor set 绑定写死了 heap 和 index

**位置**：`CommandListVK_RootSignature.cpp:89-95`（graphics）、`:193-199`（compute）

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:94` 与 `:198`（`{RHIDescriptorHeapType::Standard,0}` 写死）；`layoutInfo`/`tableParam` 取出未用（`:89-90,:193-194`）**

**现象**：
```cpp
        const DescriptorSetLayoutInfo& layoutInfo = m_pCurrentGraphicsRS->GetDescriptorSetLayout(slot.DescriptorSetIndex);
        const RootParameterDesc& tableParam = m_pCurrentGraphicsRS->GetRootParameterDesc(rootParameterIndex);

        // TODO : Implement this method
        DescriptorSetBinding binding;
        if (!pHeap->GetDescriptorSetBinding({RHIDescriptorHeapType::Standard,0},binding)) return;   // ← 写死类型 + 写死 0
```

`layoutInfo`、`tableParam` 取出完全没用，`offsetInDescriptorsFromTableStart`（入参）也被丢弃。

**影响**：只要一个 PSO 里有**两个 descriptor table**，或者表起点不是 0，两次绑定都会绑到同一份 set → 采样到错误资源，且不报错。

**建议修法**：用 `RootParameterDesc` 里的堆类型（而不是常量 `Standard`），用 `offsetInDescriptorsFromTableStart` 定位到具体 `DescriptorSetBinding`；`layoutInfo` 用于校验 binding 数量是否与 layout 匹配。

---

### P1-3 采样器完全不可用

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:110-117`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:112`（`return nullptr;`）、`:116`（`DeleteSamplerState` 空体）**

```cpp
    std::shared_ptr<RHISamplerState> DeviceVulKan::CreateSamplerState(const SamplerStateDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeleteSamplerState(std::shared_ptr<RHISamplerState>& samplerState)
    {
    }
```

**影响**：任何带 `SamplerState` 的 texture descriptor 写进去都是空 `VkSampler`；D3D12 侧 `CreateSamplerState`（`RHIResourceD3D12.cpp:138-163`）正常，所以这是两后端行为不一致，而不只是缺功能。

**建议修法**：`vkCreateSampler`，从 `SamplerStateDesc` 转 `filter`（`min/mag/mipmap`）、`addressMode`（U/V/W）、`maxAnisotropy`、`compareOp`、`borderColor`；`DeleteSamplerState` 里 `vkDestroySampler` 并把 `shared_ptr` 置空。

---

### P1-4 / P1-5 / P1-8 状态类与清除接口两侧都空

#### P1-4 VK 输出合并状态 —— **未修复**

**位置**：`CommandListVK_OutputMerger.cpp:113-121`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:115`（`OMSetBlendState` TODO）、`:120`（`OMSetDepthStencilState` TODO）**

#### P1-5 VK 深度模板清除 —— **未修复**

**位置**：`CommandListVK_Clear.cpp:36-38`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:37-38`（`ClearDepthStencilView` 空体）**

#### P1-8 D3D12 输出合并状态 —— **未修复**

**位置**：`D3D12RHI/Private/CommandList/CommandListDX12_OutputMerger.cpp:32-40`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:34`（`OMSetBlendState` TODO）、`:39`（`OMSetDepthStencilState` TODO）**

**判断**：这一组属于「接口已定义、实现留空」的半成品。当前 HelloTriangle 不需要它们；一旦要做 3D 场景（深度测试、透明混合）就会立刻撞墙。

**建议修法**：D3D12 的 `OMSetBlendState`/`OMSetDepthStencilState` 本身只是记录状态（D3D12 里 blend/depth 属于 PSO，命令级只能改 blend factor 和 stencil ref），所以实现应该是「缓存到 `m_CurrentBlendFactor` / `m_CurrentStencilRef`，供后续 `OMSetRenderTargets` 使用」；Vulkan 侧同理，仅缓存 + 断言，或在接口层标注「不支持的动态状态」。`ClearDepthStencilView` 两侧都应转 `vkCmdClearAttachments` / `ClearDepthStencilView` 并映射 clear flags。

---

### P1-6 / P1-7 D3D12 状态转换函数是空壳

**位置**：`D3D12RHI/Private/RHIPipelineStateD3D12.cpp:25-53`，调用点 `:146,148,149`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:30`（Blend TODO）、`:40`（Rasterizer TODO）、`:50`（DepthStencil TODO）；调用点 `:146`（BlendState）、`:148`（RasterizerState）、`:149`（DepthStencilState）**

```cpp
        D3D12_BLEND_DESC ConvertBlendState(RHIBlendState* blendState)
        {
            ...
                // TODO: Blend state data initialization
        }
```

三个函数都只做了「判空 + 返回默认值」，真正的字段转换（`BlendEnable`/`SrcBlend`/`DestBlend`/`BlendOp`、`FillMode`/`CullMode`/`FrontCounterClockwise`/`DepthBias`、`DepthEnable`/`DepthFunc`/`StencilEnable`）全部未做。

**P1-7 证据**：`RHIPipelineStateD3D12.cpp:162` `psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;`（旧行号 `:272-273` 已漂移至 `:162`）。

**影响**：D3D12 能画出三角形，纯粹是默认值恰好够用。一旦上层传非默认的 `RHIBlendState`/`RHIRasterizerState`，**静默失效**——上层以为设了，实际没设。

**建议修法**：按 RHI 枚举 → D3D12 枚举一一映射，注意 `RHIBlendFactor`/`RHICullMode` 不要直接 `static_cast`（两边数值顺序通常不同，建议 `switch` 显式映射）；`PrimitiveTopologyType` 从 `desc.PrimitiveTopology` 推导（POINT→POINT、LINE→LINE、TRIANGLE→TRIANGLE、PATCH→PATCH）。

---

### P1-9 常量缓冲区初始化越界读（两个后端同源）

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:126,144`、`D3D12RHI/Private/RHIResourceD3D12.cpp:177,186`

**当前状态：未修复 | 复核 2026-11-05 | 证据 VK：`:126`（256 对齐改写 `desc.SizeInBytes`）、`:144`（`memcpy(..., desc.SizeInBytes)`）；D3D12：`:177`（对齐）、`:186`（`memcpy(..., desc.SizeInBytes)`）**

**现象**：`CreateBuffer` 先按常量缓冲区规则把 `desc.SizeInBytes` 对齐到 256，这一步改的是**引用传入的调用方 desc**；随后初始化数据时又用这个**已放大的**尺寸做 `memcpy`。调用方给的 `InitialData` 只按原尺寸分配，于是最多多读 255 字节。

**影响**：堆越界读（常读到无关数据），理论上可触发崩溃；UB，调试运行时不保证复现。

**建议修法**：进入函数先保存 `const uint64_t providedBytes = desc.SizeInBytes;`，对齐只用于**资源创建尺寸**；`memcpy` 用 `providedBytes`，对齐出来的尾部按需 `memset` 0。两个后端都要改。

---

### P1-10 push-constant 合并后仍用旧索引

**位置**：`VulKanRHI/Private/RHIRootSignatureVulKan.cpp:324-329,342`；消费端 `CommandListVK_RootSignature.cpp:149,253`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:324`（临时列表下标）、`:327`（`slot.PushRangeIndex = pushRangeIndex`）、`:329`（`PushConstantBaseOffset32 = 0`）、`:342`（`MergePushConstantRanges`）；消费端 `:149,:253`（按旧下标 `GetPushConstantRange`）**

**现象**：每个 `RootParameterType::Constants` 参数在**合并前**把自己的下标存进 slot：
```cpp
    const uint32_t pushRangeIndex = static_cast<uint32_t>(temp_pushConstantRanges.size());
    temp_pushConstantRanges.push_back(pushRange);
    slot.PushRangeIndex = pushRangeIndex;              // :327 记录的是「临时列表」下标
```
随后 `:342` 执行合并，把同 stageFlags 的 range 合并、重排 offset。绑定时却拿旧下标去查新数组：`GetPushConstantRange(slot.PushRangeIndex)`。`:329` 每个参数都从 `offset=0` 起算，合并后本应错开却都写 0。

**影响**：只要有两个及以上 Constants 参数（或合并让数组变短），就会取到错误的 range —— `offset`/`size`/`stageFlags` 全错，`vkCmdPushConstants` 写到错误偏移甚至越界；常量互相覆盖。

**建议修法**：`MergePushConstantRanges` 返回时同步产出「旧下标 →（新下标, 新内偏移）」映射，回写到每个 slot 的 `PushRangeIndex` / `PushConstantBaseOffset32`；不要在 slot 里长期保存临时下标。

---

### P1-11 纹理/采样器描述符只有槽位、没有真正的 view

**位置**：`VulKanRHI/Private/DescriptorViewVulKan.cpp:90-108`

**当前状态：未修复 | 复核 2026-11-05 | 证据 纹理入口 `:97`（`Allocate()`）；采样器入口 `:107`（`Allocate()`）**

**现象**：
```cpp
    RHIDescriptorHandle DeviceVulKan::CreateStandardHeapDescriptorView(RHITexture* Texture, DescriptorRangeType Type)
    {
        if (!m_pStandardHeap || m_pStandardHeap->IsFull()) return RHIDescriptorHandle();
        return m_pStandardHeap->Allocate();     // 只分配槽位，Texture / Type 完全没用
    }
```
采样器入口 `:100-108` 同样只 `Allocate()`。对比同文件 buffer 版本：CBV `:45-51`、SRV `:58-64`、UAV `:70-76` 确实 `new` 视图对象并 `Set*Descriptor`。

**影响**：纹理 SRV/UAV 的句柄是「有效外观的空槽」——`GetDescriptorHeapView` 能取到槽，但里面 `ViewType == RRT_None`，写到 set 里就是空 `imageView`。与 P1-3 叠加，Vulkan 侧目前**完全没有可用的纹理采样路径**。

**建议修法**：创建 `VkImageViewCreateInfo`（`viewType` 由 `desc` 的 1D/2D/3D/Cube 决定、`format` 取 `VkFormat`、`subresourceRange` 按 mip/layer）→ `vkCreateImageView` → 包成 `ShaderResourceViewVulKan` → `SetSRVDescriptor(handle, view)`；采样器同理 `vkCreateSampler`。失败时 `Free(handle)` 并返回空句柄。

---

### P1-12 Buffer SRV 的 descriptor type 与 usage 不匹配

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:37-38`、`DescriptorHeapVulKan.cpp:275`

**当前状态：未修复 | 复核 2026-11-05 | 证据 usage：`:37-38`（SRV 仅 `STORAGE_TEXEL_BUFFER_BIT | UNIFORM_TEXEL_BUFFER_BIT`，无 `STORAGE_BUFFER_BIT`）；写入：`:275`（`VK_DESCRIPTOR_TYPE_STORAGE_BUFFER`）**

```cpp
    if (EnumHasAnyFlags(bindFlags, BufferBindFlag::ShaderResource))
        usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;  // 没有 STORAGE_BUFFER_BIT
...
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;   // DescriptorHeapVulKan.cpp:275
```

**影响**：`VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` 要求 buffer 带 `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`。校验层会报 `VUID-VkWriteDescriptorSet-descriptorType-00330` 一类错误；不开校验层则是「绑上了但读不到/读垃圾」。注意 UAV 路径（`:39-40`）带了 `STORAGE_BUFFER_BIT`，与 `:319` 的 `STORAGE_BUFFER` 是匹配的，问题只在 SRV。

**建议修法**：二选一——(a) 把 SRV 写成 `VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER` 并配 `VkBufferView`；(b) 给 `ShaderResource` 补 `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT` 保持 `STORAGE_BUFFER` 语义。选哪个取决于 RHI 层约定的是 structured buffer 还是 typed buffer，建议在 `BufferBindFlag` 注释里写死。

---

### P1-13 / P1-14 / P1-15 / P1-16 / P1-17 D3D12 侧的五处正确性问题

| 编号 | 状态 | 位置 | 现象 | 建议 |
|---|---|---|---|---|
| P1-13 | 未修复 | `DescriptorViewD3D12.cpp:91-99,101-109` | `CreateStandardHeapDescriptorView(RHITexture*, Type)` 从 `m_pSamplerHeap` 分配（`:93`），`Type` 未使用（`:98`）；`CreateSamplerHeapDescriptorView:103-108` 也只占槽不建 sampler | 按 `Type` 选堆并 `CreateShaderResourceView`/`CreateUnorderedAccessView`；采样器走 `CreateSampler` |
| P1-14 | 未修复 | `CommandListDX12_Resource.cpp:9-10` | `static_cast<BufferD3D12*>(pDstResource)` / `:10` 源指针，无类型校验 | 先判 `RHIResourceType` 分派到 Buffer/Texture，或在入口拒绝 |
| P1-15 | 未修复 | `CommandListDX12_InputAssembly.cpp:97,99` | `view.BufferLocation` 用基址（`:97`）、`SizeInBytes` 用整缓冲（`:99`），`pOffsets` 未使用 | `Location = 基址 + pOffsets[i]`、`SizeInBytes = 总大小 - pOffsets[i]` |
| P1-16 | 未修复 | `CommandListDX12_Clear.cpp:14-17` | `ClearDepthStencilView` 只有 `// TODO` | 调 `ClearDepthStencilView(dsv, flags, depth, stencil, 0, nullptr)`；`RHIClearFlags` 映射到 `D3D12_CLEAR_FLAG_DEPTH/STENCIL` |
| P1-17 | 未修复 | `RHIResourceD3D12.cpp:173-175` | `flag == BufferBindFlag::ConstantBuffer` 用相等（`:174`） | 改 `EnumHasAnyFlags(flag, BufferBindFlag::ConstantBuffer)`（VK 侧 `RHIResourceVulKan.cpp:122-124` 已是正确写法，可对照） |

> 补充（P1-13）：D3D12 真正的采样器创建在 `RHIResourceD3D12.cpp:138-163`（`CreateSamplerState` 调 `CreateSampler` 并占 sampler 堆槽），是可用的；本条针对的是 `DescriptorViewD3D12` 里按 `SamplerStateDesc` 走的那条占位入口。

---

### P1-18 深度屏障的 aspect 恒带 stencil

**位置**：`VulKanRHI/Private/CommandList/CommandListVK_ResourceBarrier.cpp:95-103`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:98-100`（涉及深度即返回 `DEPTH_BIT|STENCIL_BIT`，不看 format）**

```cpp
    if (涉及 DepthWrite/DepthRead)
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;  // 不看真实 format
```

**影响**：`VK_FORMAT_D16_UNORM`、`VK_FORMAT_D32_FLOAT`（无模板）用 stencil aspect 做 barrier 是非法的，校验层报 `VUID-VkImageMemoryBarrier-image-03319`；不开校验层则是偶发 undefined behavior。

> 现状补充：格式感知的 aspect 判断在 `CommandListVK_Resource.cpp:8-21`（`GetImageAspectMask`：D16/D32 仅 `DEPTH_BIT`，D24S8/D32S8 才带 stencil）已经存在，但**没有被 barrier 复用**；`ResourceBarrier` 仍用上面这个无条件版本。

**建议修法**：把 `VkFormat` 放进 `TextureVulKan`，在 barrier 的 `GetAspectMask` 里按 format 判断（可直接复用 `CommandListVK_Resource.cpp:8-21` 的helper）。参数只有状态、没有资源，建议改成传 `TextureVulKan*`（调用处 `:140` 已拿到纹理指针，可取 format）。

---

### P1-19 SPIR-V 反射把 push constant 当普通 descriptor

**位置**：`RHI/Private/RHIShaderCompiler/RHISPIRVGenerationReflection.cpp:156-172`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:163`（读 `DecorationDescriptorSet`）、`:164`（读 `DecorationBinding`）、`:170`（`resourcesBySet[res.set].push_back(res)`）**

**现象**：push constant 循环里读 `DecorationDescriptorSet` / `DecorationBinding` 并 `reflection.resourcesBySet[res.set].push_back(res)`。push constant 按 SPIR-V 规范**没有** set/binding decoration，`get_decoration` 落到默认 0 → 被塞进 `set 0` 里、和真实资源撞号。

**影响**：上层按 `set/binding` 找资源时会命中错的条目（比如把 CB 认成 push constant），D3D12 侧生成的根签名参数也会多出一组假资源。

**建议修法**：push constant 走独立类别（`reflection.pushConstants`），只取 `type_id` 的 `declared_struct_size` 和成员偏移，不进 `resourcesBySet`。

---

### P1-20 Release 下编译器状态机的空指针

**位置**：`RHI/Private/RHIShaderCompiler/RHICompilePipeline.cpp:50-56`（`BeginCompiler`）、`:66-72`（`EndCompiler` 同构）

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:51-53`（判空仅在 `#if RHI_ENABLE_DEBUG_INFO` 内）、`:56`（Release 下解引用 `controller`）；`EndCompiler`：`:67-69`、`:71-72`**

```cpp
    if (!controller) {
#if RHI_ENABLE_DEBUG_INFO
        ThrowErrorMessage("Compiler context controller is null");
#endif
    }
    if(controller->GetCompilerPipelineState() != ...)   // :56 Release 下 controller 可能就是 nullptr
```

**影响**：Debug 能报错，Release 直接空指针解引用崩溃。同文件 `Compile`（`:84-90`）与 `Reflection`（`:142-148`）在判空后有 `return {}`，写法是对的；`BeginCompiler`/`EndCompiler` 两处仍是旧的危险写法。

**建议修法**：`if (!controller) { ThrowErrorMessage(...); return false/return; }`，把 `return` 移到 `#if` 外面。全项目搜一遍同构写法（`if (!x) { #if DEBUG ThrowErrorMessage #endif }` 后面继续用 `x`）。

---

### P2-1 / P2-2 Vulkan 帧同步只有 1 个 fence

**位置**：`VulKanRHI/Internal/RHICommandListVulKan.h:224`（`VkFence m_inFlightFence`，创建于 `:172` 且带 `SIGNALED` 位）、`RHICommandListVulKan.cpp:187,230,241`

**当前状态：未修复 | 复核 2026-11-05 | 证据 单 fence 成员 `:224`；提交 `:230`（`vkQueueSubmit(..., m_inFlightFence)`）；等待 `:241`（`vkWaitForFences(1, &m_inFlightFence, ...)`）；提交前重置 `:187`**

**现象**：`BeginFrame` 只 `vkWaitForFences(1, &m_inFlightFence, ...)`，`ExecuteCommandLists` 用同一个 fence 做 `vkQueueSubmit` 的完成信号。交换链有 3 张图，`m_RenderFinishedSemaphores` 也按 `imageIndex` 建了 3 个（`:225`），但 **CPU 侧只有 1 个 fence**，第 2、3 帧无法与第 1 帧重叠。

**P2-2 证据**：`RHICommandListVulKan.cpp:191-204` 在 `SwapChainBackBufferCount` 变化时 `resize` 信号量数组，旧信号量没有 `vkDestroySemaphore` 就被覆盖；信号量按 `:225` 的 `imageIndex` 取用。

**影响**：三缓冲退化成单缓冲（延迟上升、跑不满 GPU）；`RHI_MULTI_BUFFERING=3` 在 Vulkan 路径上等于没生效；窗口来回改尺寸会漏信号量。

**建议修法**：改成 `std::vector<VkFence> m_inFlightFences`（大小 = 后备缓冲数），`BeginFrame` 等 `m_inFlightFences[m_CurrentFrameIndex]`，提交时 signal 同一个，`EndFrame` 里 `(m_CurrentFrameIndex+1) % N`。这与 D3D12 现有的 `m_FrameFenceValues[]` 思路一致。信号量 resize 时先 `vkDestroySemaphore` 旧条目。

---

### P2-3 D3D12 fence 记账时序错位

**位置**：`D3D12RHI/Private/RHICommandListD3D12.cpp:229,240,251`

**当前状态：未修复 | 复核 2026-11-05 | 证据 提交写值 `:229`（`m_FrameFenceValues[m_CurrentFrameIndex] = m_CurrentFenceValue`）；BeginFrame 读槽 `:240`；EndFrame 才换槽号 `:251`（`m_CurrentFrameIndex = m_SwapChainInfo.imageIndex`）**

**影响**：能跑（因为每帧都覆盖写），但 `m_FrameFenceValues[i]` 的语义变成「i 槽上一次提交的值」，而 `BeginFrame` 读同一个槽——两者错开一帧，一旦以后把 `BeginFrame` 改成更严格的等待策略就会变成偶发 hang。

**建议修法**：把 imageIndex 的获取提前到 `BeginFrame`（`AcquireNextImage` 之后立刻 `m_CurrentFrameIndex = imageIndex`），`ExecuteCommandLists` 里再写本帧的 fence 值。

---

### P2-4 资源创建内部同步等待 GPU

**位置**：`D3D12RHI/Private/RHIResourceD3D12.cpp:261`、`VulKanRHI/Private/RHIResourceVulKan.cpp:202`

**当前状态：未修复 | 复核 2026-11-05 | 证据 D3D12 `:261`（`m_CopyQueue...->WaitForGPU()`）；VK `:202`（`GetBasicCommandQueue(Copy)->WaitForGPU()`）**

两个后端的 `CreateBuffer` 在带 `InitialData` 时，都是「建 staging/upload → 录制拷贝 → 提交 → `WaitForGPU()`」，且这个等待会阻塞主线程。

**影响**：加载期每创建一个大 buffer 就全球串行一次；不能批量上传，也不能和别的加载工作重叠。

**建议修法**：拆两段——`CreateBuffer` 只创建资源并返回一个「上传请求」句柄，真正的拷贝在帧内统一的 upload pass 里批量录进同一个 copy command list，帧末一次性等 fence（staging ring + deferred upload）。

---

### P2-5 Vulkan 无 sub-allocation

**位置**：`VulKanRHI/Private/RHIResourceVulKan.cpp:94`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:94`（`vkAllocateMemory` 每个 buffer 一次，见 `CreateBufferAndMemory:89-100`）**

**影响**：`vkAllocateMemory` 有设备级上限（常见 4096 左右 `maxMemoryAllocationCount`），大量小 buffer 会直接分配失败；每份分配都按 `memoryType` 对齐，显存浪费明显。D3D12 侧用 `CreateCommittedResource` 也是同构问题，但 D3D12 committed resource 内部有驱动分配器兜底，Vulkan 没有。

**建议修法**：引入按 `memoryTypeIndex` 分桶的线性分配器（每桶一个 `VkDeviceMemory` 大块 + free list），`CreateBuffer` 从桶里切一段偏移，`vkBindBufferMemory(..., offset)`；buffer 销毁时归还 free list。

---

### P2-6 动态渲染附件列表不清空

**位置**：`CommandListVK_OutputMerger.cpp:104-111`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `EndDynamicRendering:109-110`（只 `vkCmdEndRenderingKHR` + `m_IsDynamicRendering=false`，无 `m_CurrentRenderTargets.clear()`）；追加在 `:45`**

```cpp
    void CommandListVulKan::EndDynamicRendering()
    {
        ...
        m_pfnCmdEndRenderingKHR(m_CommandBuffer);
        m_IsDynamicRendering = false;   // 没有 m_CurrentRenderTargets.clear()
    }
```

`OMSetRenderTargets`（`:45`）是 `push_back` 追加；`ResetFrameCaches`（`RHICommandListVulKan.cpp:171-177`）才 clear，且只在 `BeginRecording` 调。

**影响**：一帧里调两次 `OMSetRenderTargets`（先画 GBuffer、再画 backbuffer），第二次之后 `m_CurrentRenderTargets` 是 `[旧, 新]`；此时 `ClearRenderTargetView`（`CommandListVK_Clear.cpp:16`）按 `==` 找索引，会命中**旧**那个元素的下标，把 clear 打到错误的附件槽。

**建议修法**：`EndDynamicRendering` 里 `m_CurrentRenderTargets.clear(); m_CurrentRenderArea = {};`，让附件列表严格跟随当前 rendering scope 生命周期。

---

### P2-7 / P2-8 交换链与命令缓冲的生命周期

#### P2-7 交换链重建 —— **部分修复**

**位置**：`VulKanRHI/Private/SwapChainVulkan.cpp:147-158,169-192`、`D3D12RHI/Private/SwapChainD3D12.cpp:153-174`；Present 侧 `RHICommandListVulKan.cpp:270-273`、`RHICommandListD3D12.cpp:262-273`

**当前状态：部分修复 | 复核 2026-11-05 | 缺的一半：Present 返回过期/遮挡时不触发重建**

**已补上的一半**：
- VK：`Resize(width,height)`（`:169-192`）先 `vkDeviceWaitIdle`，清 RTV/backbuffer，销毁旧链后 `VkCreateSwapChain` + 重建 RTV；`AcquireNextImage` 在 `VK_ERROR_OUT_OF_DATE_KHR` 时调 `Resize` 并重新 acquire（`:147-158`）。
- D3D12：`Resize`（`:153-174`）清 RTV/backbuffer 后 `ResizeBuffers`（`:164`）再 `CreateRenderTargetView`；测试在 `WM_SIZE` 里调用（`Tests/TestSwapChain/TestSwapChain.cpp:137-142`、`Tests/HelloTriangleTest/HelloTriangleTest.cpp:341`）。

**仍缺的一半**：
- VK `Present`（`RHICommandListVulKan.cpp:270-273`）对 `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR` 只 `WarningCapture`，不重建；
- D3D12 `Present`（`RHICommandListD3D12.cpp:262-273`）只识别 `DEVICE_REMOVED/RESET`，对 `DXGI_ERROR_OUT_OF_DATE` / `DXGI_STATUS_OCCLUDED` 等无处理。
- 另外 VK `Resize` 重建出的新 backbuffer 在 `VkCreateSwapChain:329` 把 `FirstUseOfSwapChainImage` 复位为 `true`，这一点与 barrier 配合是对的。

**建议修法**：在 `Present` 的返回结果里识别过期/遮挡，走与 `Resize` 相同的重建路径；等 GPU 空闲 → 销毁 RTV 包装（不销毁 swapchain 拥有的 image）→ 建新链 → 重新取 images + 建 view。

#### P2-8 命令缓冲不复用 —— **未修复**

**位置**：`RHICommandListVulKan.cpp:95-103`、`Internal/RHICommandListVulKan.h:62`

**当前状态：未修复 | 复核 2026-11-05 | 证据 `:100`（`commandBufferCount = 1`）、`:103`（`vkAllocateCommandBuffers`）；析构 `:62`（`= default`，不释放）**

`DeviceVulKan::CreateCommandList`（`:86-112`）每次 `vkAllocateCommandBuffers(count=1)`。虽然 `CommandPoolVulKan` 析构（`RHICommandListVulKan.cpp:115-122`）会 `vkDestroyCommandPool` 连带回收，不会真泄漏，但每帧新建的分配开销和 `RESET_COMMAND_BUFFER_BIT` 的代价都白付了。建议按 pool 预分配固定数量 primary buffer 循环使用。

---

### P2-9 ~ P2-19 资源生命周期、并发与边界（补充条目）

- **P2-9 descriptor set 只回收槽位 —— 未修复**：`DescriptorHeapVulKan.cpp:111-115` 只 `Release()` 本地记录并把槽位归还 `m_FreeList`；全后端无 `vkFreeDescriptorSets` / `vkResetDescriptorPool` 调用点（已检索）。**修法**：定期 `vkResetDescriptorPool`（等 GPU 空闲）或改用 `FREE_DESCRIPTOR_SET` 池 + `vkFreeDescriptorSets`。
- **P2-10 Vulkan 关闭顺序 —— 未修复**：`RHIVulKan.cpp:886-889` 先 reset 4 个堆，`:891-893` reset 三个队列，`:905` 才 `vkDeviceWaitIdle`。**修法**：把 `vkDeviceWaitIdle` 提到所有 `reset()` 之前（或先 reset 队列再 reset 堆）。
- **P2-11 D3D12 关闭顺序 —— 未修复**：`RHID3D12.cpp:278-281` 先 reset 堆，`:283-285` reset 队列；队列析构（`RHICommandListD3D12.cpp:203`）才 `WaitForGPU`。**修法**：`Shutdown` 开头对三个队列显式 `WaitForGPU()`，再销毁堆。
- **P2-12 根签名 range 泄漏 —— 未修复**：`RHIRootSignatureD3D12.cpp:45` `new D3D12_DESCRIPTOR_RANGE[count]`，全文件无 `delete[]`。**修法**：`std::vector<D3D12_DESCRIPTOR_RANGE>` 或 `unique_ptr<T[]>`，序列化返回后自动释放（D3D12 序列化是立即拷贝的，不持有指针，可以安全释放）。
- **P2-13 Unmap 零写入范围 —— 未修复**：`RHIResourceD3D12.h:85-86` `D3D12_RANGE writeRange(0, 0); m_pResource->Unmap(0, &writeRange);` 等于告诉驱动「我一个字节都没写」。**修法**：`Map` 时记录本次映射允许写入的字节数，`Unmap` 传 `D3D12_RANGE(0, written)`；或对 UPLOAD 堆直接传 `nullptr`。
- **P2-14 Readback 无非一致失效 —— 未修复**：`RHIResourceVulKan.cpp:19-20` 只要求 `HOST_VISIBLE|HOST_CACHED`。**修法**：读之前 `vkInvalidateMappedMemoryRanges(device, 1, &range)`，或在 `FindMemoryType` 里追加 `HOST_COHERENT` 并处理找不到的情况。
- **P2-15 JobSystem 遗弃任务 —— 未修复**：`JobSystem.cpp:71` 置 `running=false` 后 `:74-78` join；`WorkerLoop:162-170` 的 `while(running)` 使工作线程最多再跑 1 个任务就退出，剩余队列无人消费。而 `Wait:130-136` 是 `while(counter>0) { TryRunOne(); else yield(); }` → **无限 yield 空转**。**修法**：`Uninstall` 里先 `running=false` 阻止新任务入队，再循环 `TryRunOne()` 排空队列，最后 join。
- **P2-16 ScheduleBatch 持锁调工厂 —— 未修复**：`JobSystem.cpp:102-106` 在 `lock_guard` 作用域内执行 `factory(i)`（`:104`）。**修法**：先在锁外把 `count` 个 `JobFn` 全部生成到局部 vector，再进锁批量 `push` 并一次性 `notify_all`。
- **P2-17 Buffer 边界加法溢出 —— 未修复**：`Buffer.h:54`（`m_WriteOffset + size`）、`:178`（`m_ReadOffset + count`）。**修法**：先判 `offset <= total`，再判 `size <= total - offset`。另：`BinaryReader(const BinaryWrite&)`（`:143-149`）实际已做深拷贝（`:148`），`:153` 注释仍写「共享数据」，注释需更正。
- **P2-18 MoveData 不重置元数据 —— 未修复**：`Buffer.h:108-111` 只 `ShrinkToFit()` 再 `std::move`。**修法**：移出后 `m_WriteOffset = 0; m_CurrentSize = 0;`（`m_Data` 已被移空）。
- **P2-19 D3D12 描述符视图无边界检查 —— 未修复**：`DescriptorHeapD3D12.cpp:53-54` 直接 `m_Descriptors[index]`，未判 `index < Capacity`（同文件 `Free:102`、`GetCPUHandle:24`、`GetAnyDescriptor:172` 有检查）。**修法**：补 `if (index >= Capacity) { ThrowErrorMessage(...); return nullptr; }`，与 VK 侧 `DescriptorHeapVulKan.cpp:94` 对齐。

---

### P3 代码卫生 / 构建

- **P3-1 —— 未修复**：`VK_USE_PLATFORM_WIN32_KHR` 定义在 `RHIVulKan.h:9` 的 `#define` 里，而 `RHICommandListVulKan.h:4`、`RHIResourceVulKan.h:3`、`DescriptorHeapVulKan.h:3`、`RHIRootSignatureVulKan.h:5`、`RHIFormatVulkan.h:4` 都直接 `#include <vulkan.h>`。目前能编过是因为包含顺序刚好让 `RHIVulKan.h` 先被看到；换个 TU 顺序（或预编译头介入）就可能丢掉 `vkCreateWin32SurfaceKHR` 声明。**建议移到 `VulKanRHI/CMakeLists.txt`**：
  ```cmake
  target_compile_definitions(VulKanRHI PRIVATE VK_USE_PLATFORM_WIN32_KHR VULKANRHI_EXPORTS)
  ```
- **P3-2 —— 未修复**：`CommandListVK_*` 里仍有大量 `if (...) return;` 的静默早退（例 `CommandListVK_RootSignature.cpp:73,77,141`），同时混用 `ThrowErrorMessage`。静默早退在 release 下永远不报，是这批 P0/P1 能藏这么久的原因。建议统一：debug 下全部走 `ThrowErrorMessage`，release 下至少 `Core::ErrorCapture`。
- **P3-3 —— 未修复**：`RHIPipelineStateVulkan.cpp:62-154` 构造的 `shaderStages` 未被使用（见 P0-1），`:158` 即 `return false`。
- **P3-4 —— 未修复**：`HelloTriangleTest.cpp:252-253` `ASSERT_TRUE(graphicsPSO && graphicsPSO->IsValid())` 与后端强绑，未按 `RHIType` 分支跳过。建议 Vulkan 未完成时 `GTEST_SKIP()` 并带原因，避免「一选 vk 就挂且看不出根因」。
- **P3-5 —— 未修复**：`Tests/TestSwapChain/TestSwapChain.cpp:17`（`WM_DESTROY` 里 `return 0` 不发 `WM_QUIT`）、`:86`（`while(true)`）、`:96-100`（唯一退出条件是 5 秒超时）。**影响**：用户提前关窗后，循环仍 `AcquireNextImage` 并向已销毁窗口的交换链提交，跑满 5 秒。**修法**：`WM_DESTROY` 里 `PostQuitMessage(0)`，循环里判 `msg.message == WM_QUIT` 立即 break。

---

## 3. 建议修复顺序

> 按 2026-11-05 当前状态调整；已满足的条目标注「已满足」。

1. **跑通已有构建产物并做基线验证**。`Binary\Debug\` 已有 2026-11-05 09:39 的整包产物（含 `Magic-VulKanRHI.dll`、`Magic-D3D12RHI.dll` 与三个测试 exe），但本次复核未运行；下一步先在这套产物上跑 D3D12 / Vulkan 基线，确认可启动、可出图，再继续修缺陷。顺带做 P3-1、P1-20（Release 空指针一编译就暴露）。
2. **内存安全三件套（不依赖图形链路，可以现在就修）**：P1-9（CB 对齐后越界读，两后端）、P2-17（Buffer 边界溢出）、P2-19（D3D12 描述符视图越界）。
3. **P0-1 → P0-3**：P0-2（管线绑定）、P0-4（视口/裁剪）已打通，剩余根断点是 **P0-1（图形 PSO 未创建）** 与 **P0-3（IA 空实现）**。建议同一批做，串起来才能拿到 Vulkan 第一张三角形；单独修任何一个都看不到图。
4. **P1-2 → P1-11 → P1-12 → P1-1**：先修「绑错 set」「空纹理槽位」「descriptor type 不匹配」三个硬错误，再决定 root descriptor 的长期方案（推荐先立「Vulkan 只支持 descriptor table + push constant」的约束）。
5. **P1-10 / P1-18 / P1-19**：push-constant 索引、深度 aspect、反射分类，三个都是「绑定看起来成功但值不对」的隐蔽错误，做多参数/多资源场景前必须修。
6. **P1-3 / P1-4 / P1-5 / P1-6 / P1-7 / P1-8 / P1-13 / P1-14 / P1-15 / P1-16 / P1-17**：状态类与描述符类，做 3D 场景前必须补齐；D3D12 的三个 `Convert*` 优先级高于 Vulkan 的同名空函数（D3D12 是「主路径」）。
7. **P2-1 / P2-2 / P2-3 / P2-6 / P2-15 / P2-16**：同步正确性与并发。P2-6 属正确性，提前到第 4 步；P2-15/P2-16 是 JobSystem 的死锁与空转，影响 `SubsystemControl` 退出，建议和第 2 步一起处理。
8. **P2-9 / P2-10 / P2-11 / P2-12 / P2-13 / P2-14 / P2-19**：生命周期与泄漏，稳定出图后逐个修，且必须配合校验层验证。
9. **P2-4 / P2-5 / P2-7 / P2-8 / P3-5**：性能、交换链重建与测试收尾，最后做。P2-7 已具备 `Resize` 路径，剩余 Present 侧过期/遮挡恢复并入本步。

---

## 4. 修复后的验收清单

> 状态按 2026-11-05 复核标注；「未满足」注明卡在哪个文件。

- [x] 后端 DLL 与测试可执行已生成（**已满足于构建层**：`Binary\Debug\` 2026-11-05 09:39 产物含 `Magic-D3D12RHI.dll`、`Magic-VulKanRHI.dll`、`Magic-RHI.dll`、`Magic-Core.dll` 及 `HelloTriangleTest.exe` / `TestSwapChain.exe` / `TestShaderCompile.exe`；**有构建产物，但本次复核未运行**。`out\bin\` 仅是更早的旧 D3D11/D3D12 产物）
- [ ] `HelloTriangleTest` 在 D3D12 下出三角形（基线，代码层链路基本完整，**运行时未验证**）
- [ ] `HelloTriangleTest` 在 Vulkan 下出**同样的**三角形（**未满足**：卡在 P0-1 `RHIPipelineStateVulkan.cpp:158` 与 P0-3 `CommandListVK_InputAssembly.cpp:10-20`；P0-2、P0-4 命令路径已通）
- [ ] Vulkan 校验层（`VkLayer_khronos_validation.dll` 已随 `Binary\Debug\` 提供）在整帧循环里零 error / 零 warning（校验层可用，但本次复核未启用、未运行）
- [ ] ASan（`/fsanitize=address`）下创建 1 个 256 对齐的 CB，无越界读告警（**未满足**：P1-9 两后端）
- [ ] 一个 PSO 带 2 个 Constants 参数（同 stageFlags 触发合并），两个常量都读到正确值（**未满足**：P1-10）
- [ ] 纹理 + 采样器用例：两后端采样结果一致，且 VK 侧校验层无 descriptor 相关 error（**未满足**：P1-11 / P1-12 / P1-13）
- [ ] 带偏移地绑定顶点缓冲（`pOffsets != 0`）两后端一致（**未满足**：P1-15）
- [ ] 改窗口尺寸连续 20 次不崩、不出 `OUT_OF_DATE` 死循环（**部分满足**：`Resize` 两侧已具备，Present 侧过期/遮挡恢复仍缺，见 P2-7）
- [ ] 「关背面剔除 + 开 alpha blend + 开深度测试」用例两后端出图一致（**未满足**：P1-4 / P1-6 / P1-7 / P1-8 / P1-16 / P1-17）
- [ ] 一张 PSO 带两个 descriptor table（第二个带非零偏移）用例两后端一致（**未满足**：P1-2）
- [ ] 创建 100 个 buffer 的耗时：只发生 1 次 GPU 往返（**未满足**：P2-4）
- [ ] `JobSystem::Uninstall` 后队列已排空，无 `Wait` 空转（P2-15）；`ScheduleBatch` 内工厂再 `Schedule` 不死锁（P2-16）（**未满足**）
- [ ] `TestSwapChain` 手动关窗后 1 秒内退出，不跑满 5 秒（**未满足**：P3-5）

---

## 5. 审查范围与假设

**已逐行读（本次复核覆盖）**：
- `RHI`：`RHIInterface.h`、`IRHIModule.h`、`RHICommandList.h`、`RHIResource.h`、`RHIRootSignature.h`、`RHIPipelineState.h`、`Common/RHIDefinitions.h`、`Common/RHITypes.h`、`Common/RHIConfig.h`、`RHILoader.cpp`、`RHIPlatformDetection.cpp`、`RHIShaderCompiler/*`（CompilePipeline / CompilerInstance / SPIRV 反射入口）
- `D3D12RHI`：`RHID3D12.cpp`、`RHICommandListD3D12.cpp`、`SwapChainD3D12.cpp`、`RHIResourceD3D12.cpp/.h`、`RHIRootSignatureD3D12.cpp`、`RHIPipelineStateD3D12.cpp`、`CreateShaderD3D12.cpp`、`ShaderCompilerBackendD3D12.cpp`、`DescriptorHeapD3D12.cpp`、`DescriptorViewD3D12.cpp`、`CommandListDX12_*`（全部 9 个）
- `VulKanRHI`：`RHIVulKan.cpp/.h`、`RHICommandListVulKan.cpp/.h`、`SwapChainVulkan.cpp`、`RHIResourceVulKan.cpp`、`RHIRootSignatureVulKan.cpp/.h`、`RHIPipelineStateVulkan.cpp`、`CreateShaderVulKan.cpp`、`ShaderCompilerBackendVulkan.cpp`、`DescriptorHeapVulKan.cpp/.h`、`DescriptorViewVulKan.cpp`、`CommandListVK_*`（全部 9 个）
- `Core`：`Private/Jobsystem/JobSystem.cpp`、`Public/Tools/Buffer.h`
- `Tests`：`HelloTriangleTest.cpp`、`TestSwapChain.cpp`、`TestShaderCompile.cpp`（目录结构）

**未逐行展开**：`Core` 模块其余部分（日志捕获/断言宏/`SubsystemControl` 实现）、`D3D12RHI/DescriptorViewD3D12.cpp` 的 buffer 分支细节、`SPIRV-Cross` 与 `DirectXShaderCompiler` 第三方、`SingleHeaderInit*` 的初始化聚合、`ThirdParty` 全部。

**假设**：
- `ThirdParty/VulkanSdk` 是完整 SDK（目录存在，含 `Include`/`lib`/`dll`），未校验版本。
- `Binary\Debug\` 已有 2026-11-05 09:39 的构建产物（含 `Magic-VulKanRHI.dll` 与三个测试 exe）及校验层 DLL，但本次复核未运行这些产物、未启用校验层；所有「能画出/画不出」的结论来自代码路径完整性，不是实测；barrier 合法性、内存一致性、GPU 可见性均需在真机/校验层下确认。
- 附录 A 的旧文档条目判定标准是「回源码逐条核对」，只判断缺陷是否成立，不评价旧文档的分级与措辞。

**近期已补能力（提交 8d0c383、46dbaf6、18bd47d、c410bb0，本次复核确认）**：`TextureVulKan::GetSize()` / `TextureD3D12::GetSize()` 已接入 `GetTextureSizeInBytes`（`RHIResourceVulKan.cpp:323-326`、`RHIResourceD3D12.cpp:289-292`、`RHIDefinitions.h:108-156`）；`R8_SRGB` 按 1 字节处理（`RHITypes.h:266-267`）。旧清单 48 条中无直接对应条目，此处记录为已补齐的语义能力。

---

## 附录 A：旧缺陷文档《代码缺陷.md》28 条核对结果

判定口径：**属实且新增** = 本文此前未覆盖、回源码确认成立；**已覆盖** = 与本文某条为同一缺陷；**需更正** = 旧文档描述与实际代码不符。「现状（2026-11-05）」列逐条回 HEAD `18d30c2` 复核。

| 旧编号 | 旧文档要点 | 判定 | 对应本文 | 现状（2026-11-05） |
|---|---|---|---|---|
| 1 | 任务系统卸载时会遗弃排队任务 | 属实且新增（「永久阻塞」应表述为 `Wait` 空转） | P2-15 | 未修复：`JobSystem.cpp:71-78` 仍置位后直接 join，剩余队列不清空；`Wait:130-136` 仍 yield 空转 |
| 2 | Vulkan 图形管线创建始终失败 | 已覆盖 | P0-1 | 未修复：`RHIPipelineStateVulkan.cpp:158` 仍 `return false`，无 `vkCreateGraphicsPipelines` |
| 3 | Vulkan 多个图形命令接口为空操作 | **描述有误**：`ClearRenderTargetView` 与 `OMSetRenderTargets` **都已实现**；真正空的是 `ClearDepthStencilView`、`OMSetBlendState`、`OMSetDepthStencilState` | P1-4 / P1-5 | 描述有误的判定仍成立：`ClearRenderTargetView` 见 `CommandListVK_Clear.cpp:5-34`，`OMSetRenderTargets` 见 `CommandListVK_OutputMerger.cpp:6-93`；空体为 `:113-116,:118-121` 与 `CommandListVK_Clear.cpp:36-38` |
| 4 | Vulkan 纹理/采样器描述符没有创建实际资源视图 | 属实且新增（采样器 `nullptr` 部分已覆盖） | P1-11（新增） + P1-3 | 未修复：`DescriptorViewVulKan.cpp:90-108` 仍只 `Allocate()`；`RHIResourceVulKan.cpp:112` 仍返回 `nullptr` |
| 5 | Vulkan 描述符表绑定忽略调用方指定的表位置 | 已覆盖 | P1-2 | 未修复：`CommandListVK_RootSignature.cpp:94,198` 仍写死 `{Standard,0}`，偏移被丢弃 |
| 6 | Vulkan push-constant 合并后仍用旧索引 | 属实且新增 | P1-10 | 未修复：`RHIRootSignatureVulKan.cpp:327,342` 与消费端 `:149,:253` 仍用合并前下标 |
| 7 | Vulkan 常量缓冲区初始化越界读调用方数据 | 属实且新增，**且 D3D12 侧同源**（旧文档只提 Vulkan） | P1-9 | 未修复：VK `:126,144`、D3D12 `:177,186` 仍用对齐后尺寸 `memcpy` |
| 8 | Vulkan Buffer SRV 描述符类型与用途不匹配 | 属实且新增 | P1-12 | 未修复：usage `RHIResourceVulKan.cpp:37-38` 无 `STORAGE_BUFFER_BIT`，写入 `DescriptorHeapVulKan.cpp:275` 仍 `STORAGE_BUFFER` |
| 9 | Vulkan 关闭先销毁描述符池后 `vkDeviceWaitIdle` | 属实且新增 | P2-10 | 未修复：`RHIVulKan.cpp:886-893` 先 reset 堆/队列，`:905` 才 wait idle |
| 10 | 释放槽位但不释放 descriptor set，最终耗尽池 | 属实且新增 | P2-9 | 未修复：`DescriptorHeapVulKan.cpp:111-115` 仍只清本地记录，无 `vkFreeDescriptorSets`/`vkResetDescriptorPool` |
| 11 | D3D12 关闭在 GPU 完成前销毁描述符堆 | 属实且新增 | P2-11 | 未修复：`RHID3D12.cpp:278-285` 先 reset 堆/队列，队列析构 `RHICommandListD3D12.cpp:203` 才 wait |
| 12 | D3D12 纹理描述符返回错误类型的句柄 | 属实且新增 | P1-13 | 未修复：`DescriptorViewD3D12.cpp:91-99` 仍从 sampler 堆分配 |
| 13 | D3D12 `CopyResource` 错误转换为 Buffer | 属实且新增 | P1-14 | 未修复：`CommandListDX12_Resource.cpp:9-10` 仍 `static_cast` 无类型校验 |
| 14 | 批量任务工厂持锁时调用用户代码 | 属实且新增 | P2-16 | 未修复：`JobSystem.cpp:102-106` 仍持锁调 `factory(i)` |
| 15 | `BinaryReader` 边界检查整数溢出 | 属实且新增（与 16 合并为一条） | P2-17 | 未修复：`Buffer.h:178` 仍先加后比；`BinaryReader` 深拷贝本身已做（`:148`） |
| 16 | `BinaryWrite` 容量计算溢出 | 属实且新增（与 15 合并为一条） | P2-17 | 未修复：`Buffer.h:54` 仍 `m_WriteOffset + size` 先加后比 |
| 17 | `MoveData` 移出数据后未重置元数据 | 属实且新增 | P2-18 | 未修复：`Buffer.h:108-111` 仍只 `ShrinkToFit()` + move |
| 18 | D3D12 深度/模板清除与输出合并状态为空操作 | 属实且新增（OM 部分已覆盖） | P1-16（新增） + P1-8 | 未修复：`CommandListDX12_Clear.cpp:14-17`、`CommandListDX12_OutputMerger.cpp:32-40` 仍空 |
| 19 | D3D12 顶点缓冲绑定忽略输入偏移 | 属实且新增 | P1-15 | 未修复：`CommandListDX12_InputAssembly.cpp:97,99` 仍忽略 `pOffsets` |
| 20 | D3D12 描述符视图缺少索引边界检查 | 属实且新增 | P2-19 | 未修复：`DescriptorHeapD3D12.cpp:53-54` 仍无 `index < Capacity` 检查 |
| 21 | D3D12 根签名描述符范围数组泄漏 | 属实且新增 | P2-12 | 未修复：`RHIRootSignatureD3D12.cpp:45` 仍 `new[]`，无 `delete[]` |
| 22 | D3D12 `Unmap` 报告写入范围为空 | 属实且新增（后果需按堆类型区分，见条目说明） | P2-13 | 未修复：`RHIResourceD3D12.h:85-86` 仍 `writeRange(0, 0)` |
| 23 | D3D12 CB 对齐只检查完全相等的标志 | 属实且新增 | P1-17 | 未修复：`RHIResourceD3D12.cpp:174` 仍用 `==` 比较 |
| 24 | Vulkan Readback 映射未处理非一致性内存失效 | 属实且新增 | P2-14 | 未修复：`RHIResourceVulKan.cpp:19-20` 仍仅 `HOST_VISIBLE|HOST_CACHED` |
| 25 | Vulkan 深度屏障为无模板格式带 stencil aspect | 属实且新增 | P1-18 | 未修复：`CommandListVK_ResourceBarrier.cpp:98-100` 仍无条件带 stencil；格式感知 helper 在 `CommandListVK_Resource.cpp:8-21` 未被复用 |
| 26 | SPIR-V 反射把 push constant 当普通 descriptor | 属实且新增 | P1-19 | 未修复：`RHISPIRVGenerationReflection.cpp:163-164,170` 仍读 set/binding 并入 `resourcesBySet` |
| 27 | 编译器 Begin/End 在 Release 下空指针 | 属实且新增 | P1-20 | 未修复：`RHICompilePipeline.cpp:50-56,66-72` 判空仍在 `#if` 内 |
| 28 | 交换链测试关窗后仍继续提交渲染 | 属实且新增 | P3-5 | 未修复：`TestSwapChain.cpp:17,86,96-100` 仍无 `WM_QUIT`、靠 5 秒超时退出 |

> 合计：旧文档 28 条中 **25 条属实且为本文遗漏项**（第 15、16 条合并为 P2-17），**2 条与本文重复**（第 2、5 条），**1 条描述错误**（第 3 条）。本文条目总数仍为 **48 条**。

---

## 新增（未编号，复核于 2026-11-05）

> 以下为本次复核在 48 条之外新发现的问题，暂不分配 P0~P3 编号，待后续评估定级。证据均为 HEAD `18d30c2` 静态代码位置。

- **N1 每描述符一条 `VkDescriptorSetLayout` 泄漏**：`DescriptorHeapVulKan.cpp:179-191`（`AllocateDescriptorSet`）为每个描述符新建一条 1-binding 的 `VkDescriptorSetLayout`，存入 `binding.layout`；`Free`（`:111-115`）只 `Release()` 绑定，堆析构（`:50-57`）只 `vkDestroyDescriptorPool`，**这条 layout 从不销毁**。对照：根签名的 layout 在 `RHIRootSignatureVulKan.cpp:382` 有 `vkDestroyDescriptorSetLayout`，堆内这条没有。长期反复分配/释放会持续泄漏 descriptor set layout。
- **N2 计算管线局部 `VkPipelineLayoutCreateInfo` 死代码**：`RHIPipelineStateVulkan.cpp:210-213` 构造了 `layoutInfo`（`setLayoutCount=0`、`pushConstantRangeCount=0`，两处 TODO），但 `:219` 实际传给 `vkCreateComputePipelines` 的是 `rootSig->GetPipelineLayout()`；局部 `layoutInfo` 从未被使用。易误读为「计算管线用了独立 layout」。
- **N3 Vulkan buffer GPU 虚拟地址恒为 0**：`RHIResourceVulKan.cpp:296-299` `BufferVulKan::GetGPUVirtualAddress()` 直接 `return 0`。Vulkan root CBV/SRV/UAV 按 GPU 地址取值的路径（P1-1）即便补实现，也缺少可用的地址来源；需明确 RHI 层在 VK 侧只支持 descriptor table。
- **N4 `ResetFrameCaches` 未清当前 PSO 缓存指针**：`RHICommandListVulKan.cpp:171-177` 重置了 RS 指针与渲染附件缓存，但 `m_pCurrentGraphicsPSO` / `m_pCurrentComputePSO`（`Internal/RHICommandListVulKan.h:150-151`）未置空。跨录制复用命令列表时，上一帧的 PSO 残留可能被后续逻辑误用；建议在 `ResetFrameCaches` 一并置空。
- **N5 测试按 `ImageIndex` 索引命令列表，缺边界保护**：`Tests/TestSwapChain/TestSwapChain.cpp:104,108` 用 `ImageIndex` 直接索引 `cmdPools[ImageIndex]` / `cmdLists[ImageIndex]`，未校验 `ImageIndex < RHI_MULTI_BUFFERING`；`AcquireNextImage` 返回 `UINT32_MAX`（失败，见 `SwapChainVulkan.cpp:162`）时会越界。`HelloTriangleTest` 同构用法需一并检查。
