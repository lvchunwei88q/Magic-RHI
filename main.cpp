#include <Subsystem/Subsystem.h>
#include <iostream>
#include <windows.h>
#include <cstring>

#include <CoreMinimal.h>
#include <RHI.hpp>
#include <IO.h>

#define PI 3.1415926f

uint32_t x, y; // windows size 
bool ExeSetSize = false;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
    {
        // 获取新的窗口大小
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        if (width > 0 && height > 0)
        {
            x = width;
            y = height;
            ExeSetSize = true;
        }
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

#include <random>
#include <chrono>

void PrintReflection(const RHI::SPIRVReflection& reflection) {
    std::cout << "\n========== SPIR-V Reflection Info ==========" << std::endl;
    std::cout << "Entry Point: " << reflection.entryPoint << std::endl;
    std::cout << "Total Constant Buffer Size: " << reflection.totalConstantBufferSize << " bytes" << std::endl;
    std::cout << "Total Resources: " << reflection.resources.size() << std::endl;
    
    // 按 Space (set) 分组打印
    for (auto& [set, resources] : reflection.resourcesBySet) {
        std::cout << "\n  [Space " << set << "] (" << resources.size() << " resources)" << std::endl;
        for (auto& res : resources) {
            std::cout << "    - " << res.name << std::endl;
            std::cout << "      Binding: " << res.binding << std::endl;
            std::cout << "      Size: " << res.size << " bytes" << std::endl;
            
            if (res.isConstantBuffer) {
                std::cout << "      Type: Constant Buffer" << std::endl;
                if (!res.members.empty()) {
                    std::cout << "      Members:" << std::endl;
                    for (auto& member : res.members) {
                        std::cout << "        - " << member.first << " @ offset " << member.second << std::endl;
                    }
                }
            } else if (res.isTexture) {
                std::cout << "      Type: Texture" << std::endl;
            } else if (res.isSampler) {
                std::cout << "      Type: Sampler" << std::endl;
            }
        }
    }
    std::cout << "=============================================\n" << std::endl;
}

struct Vertex
{
    float Position[4];  // XYZW
    float Color[4];     // RGBA
};

std::vector<Vertex> GenerateRandomVertices(size_t targetSize)
{
    constexpr size_t vertexSize = sizeof(Vertex);  // 28 bytes
    size_t targetVertexCount = (targetSize) / vertexSize;
    
    std::vector<Vertex> vertices;
    vertices.reserve(targetVertexCount);
    
    // 随机数生成器
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> distPos(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
    
    for (size_t i = 0; i < targetVertexCount; ++i)
    {
        Vertex v;
        // 随机位置
        v.Position[0] = distPos(rng);
        v.Position[1] = distPos(rng);
        v.Position[2] = distPos(rng);
        v.Position[3] = 1.0f;
        
        // 随机颜色
        v.Color[0] = distColor(rng);
        v.Color[1] = distColor(rng);
        v.Color[2] = distColor(rng);
        v.Color[3] = 1.0f;
        
        vertices.push_back(v);
    }
    
    return vertices;
}

// 是的你没看错这是一个屎山代码😀
int main(int argc, char* argv[])
{
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;

    RHI::RHIType type = RHI::GetBestAvailableRHI();
    
    if (argc > 1 && strcmp(argv[1], "dx12") == 0) {
        type = RHI::RHIType::D3D12;
        std::cout << "Using D3D12" << std::endl;
    } else {
        type = RHI::RHIType::D3D11;
        std::cout << "Using D3D11" << std::endl;
    }

    Core::SubsystemControl::Init();

    { // 注册核心日志捕获
        Core::ErrorCapture::RegisterCaptureFunction([](const std::string& Message){
            std::cout << "Error: " << Message << std::endl;
        });
        Core::WarningCapture::RegisterCaptureFunction([](const std::string& Message){
            std::cout << "Warning: " << Message << std::endl;
        });
        Core::InfoCapture::RegisterCaptureFunction([](const std::string& Message){
            std::cout << "Info: " << Message << std::endl;
        });
    }

    const wchar_t CLASS_NAME[] = L"RHIWindowClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        type == RHI::RHIType::D3D11 ? L"RHI D3D11 Window" : L"RHI D3D12 Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (hwnd == nullptr)
    {
        std::cout << "Failed to create window!" << std::endl;
        return 1;
    }
    
    ShowWindow(hwnd, SW_SHOW);
    
    std::cout << "Initializing Device..." << std::endl;

    RHIAPILoader* loader = RHIModule::GetRHILoader();

    RHIShaderCompiler* compilerContextController = RHIModule::GetCompilerContextController();
    RHIShaderCompiler* SPIRVcompiler = RHIModule::GetSPIRVCompiler();
    RHIShaderCompiler* HLSLCompiler = RHIModule::GetHLSLCompiler();
    RHIShaderCompiler* SPIRVreflector = RHIModule::GetSPIRVReflection();

    bool isInitialized = compilerContextController->InitializeCompilerContext();
    if (!isInitialized) {
        std::cerr << "Failed to initialize shader compiler context!" << std::endl;
        return 1;
    }

    if (!SPIRVcompiler || !SPIRVreflector || !HLSLCompiler) {
        std::cerr << "Failed to get SPIR-V processor or HLSL processor!" << std::endl;
        return 1;
    }

    loader->Load(type);
    std::cout << "Is Multi-Threading Supported: " << (RHI::IsMultiThreadingSupported(loader->GetRHIType()) ? "Yes" : "No") << std::endl;
    
    auto device = loader->CreateDevice();
    if (device && device->Initialize())
    {
        std::cout << "Feature Level: " << GetFeatureLevelName(device->GetFeatureLevel()) << std::endl;
        std::cout << "Shader Model Version: " << ShaderModelToString(device->GetShaderModelVersion()) << std::endl;
        std::wcout << L"Device initialized successfully: " << device->GetAdapterName() << std::endl;
        
        RHI::SwapChainDesc swapChainDesc = {};
        swapChainDesc.WindowHandle = hwnd;
        swapChainDesc.Width = 800;
        swapChainDesc.Height = 600;
        swapChainDesc.VSync = false;
        
        auto swapChain = loader->CreateSwapChain();
        if (swapChain && swapChain->Initialize(device.get(), swapChainDesc))
        {
            std::cout << "SwapChain created successfully!" << std::endl;

            uint64_t frequency = 0;
            device->GetCommandQueue(RHI::RHICmdType::Graphics)->GetTimestampFrequency(&frequency);
            std::cout << "Timestamp Frequency: " << frequency << std::endl;

            Vertex tvertices[] = {
                { { 0.0f,  0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // 顶点0: 顶部，红色
                { { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // 顶点1: 右下，绿色
                { {-0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }   // 顶点2: 左下，蓝色
            };

            std::vector<Vertex> cvertices = GenerateRandomVertices(2 * 1024);  // 2 KB
            
            auto CreateBuffer = [&](RHI::BufferHeapType t,RHI::BufferBindFlag f,RHI::DescriptorRangeType rt) {
                RHI::BufferDesc desc;
                desc.SizeInBytes = sizeof(Vertex) * cvertices.size();
                desc.Stride = sizeof(Vertex);
                desc.InitialData = cvertices.data();
                desc.HeapType = t;
                desc.BindFlags = f;

                auto buffer = device->CreateBuffer(desc);
                device->CreateStandardHeapDescriptorView(buffer.get(), rt);
                return buffer;
            };

            std::shared_ptr<RHI::RHIConstantBuffer> constantBuffer = CreateBuffer(RHI::BufferHeapType::Default,RHI::BufferBindFlag::UnorderedAccess,RHI::DescriptorRangeType::UAV);
            std::cout << "UnorderedAccess created successfully!" << std::endl;
            std::cout << "UnorderedAccess Handle: " << constantBuffer->GetBindlessHandle().GetIndex() << std::endl;
            
            std::shared_ptr<RHI::RHIConstantBuffer> constantBuffer1 = CreateBuffer(RHI::BufferHeapType::Default,RHI::BufferBindFlag::ConstantBuffer,RHI::DescriptorRangeType::CBV);
            std::cout << "ConstantBuffer created successfully!" << std::endl;
            std::cout << "ConstantBuffer Handle: " << constantBuffer1->GetBindlessHandle().GetIndex() << std::endl;

            auto cbv = device->GetDescriptorHeap(RHI::RHIDescriptorHeapType::Standard)->GetDescriptorHeapView(constantBuffer1->GetBindlessHandle());
            RHI::RHIConstantBufferView* cbvView = SafeCast<RHI::RHIConstantBufferView>(cbv);
            std::cout << "ConstantBuffer View: " << cbvView->GetGPUVirtualAddress() << std::endl;

            RHI::BufferDesc vbdesc;
            vbdesc.SizeInBytes = sizeof(Vertex) * 3;
            vbdesc.Stride = sizeof(Vertex);
            vbdesc.InitialData = tvertices;
            vbdesc.HeapType = RHI::BufferHeapType::Default;
            vbdesc.BindFlags = RHI::BufferBindFlag::VertexBuffer;
            std::shared_ptr<RHI::RHIVertexBuffer> vertexBuffer = device->CreateBuffer(vbdesc);
            std::cout << "VertexBuffer created successfully!" << std::endl;
            
            std::cout << "Shader compilation..." << std::endl;
            std::string versionStr = std::to_string(ShaderModelToNumber(device->GetShaderModelVersion()));
            // 从文件编译顶点着色器
            std::string exePath = IO::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
            std::string ShaderPath = exePath + "\\..\\..\\Test";
            std::string vsshaderPath = ShaderPath + "\\testVS.hlsl"; // 你知道的这只是一个测试示例
            std::string psshaderPath = ShaderPath + "\\testPS.hlsl";
            std::string csshaderPath = ShaderPath + "\\testCS.hlsl";
            RHI::ShaderCompileDesc vsDesc{};
            vsDesc.Type = RHI::ShaderType::Vertex;
            vsDesc.EnableDebugInfo = true;
            vsDesc.FilePath = vsshaderPath.c_str();

            RHI::ShaderCompileDesc psDesc{};
            psDesc.Type = RHI::ShaderType::Pixel;
            // Set shader model macro
            psDesc.Macros.push_back({"SHADER_MODEL", 
                versionStr.c_str()});
            psDesc.EnableDebugInfo = true;
            psDesc.FilePath = psshaderPath.c_str();

            RHI::ShaderCompileDesc csDesc{};
            csDesc.Type = RHI::ShaderType::Compute;
            csDesc.EnableDebugInfo = true;
            csDesc.FilePath = csshaderPath.c_str();

            auto vertexShader = device->CompileVertexShader(vsDesc);
            auto pixelShader = device->CompilePixelShader(psDesc);
            auto computeShader = device->CompileComputeShader(csDesc);

            // 现在测试编译 SPIR-V
            RHI::ShaderCompileOptions options;
            options.entryPoint = "main";
            options.targetProfile = "ps_6_0";
            options.optimize = true;
            options.debugInfo = true;
            options.Macros.push_back({"SHADER_MODEL", 
                versionStr.c_str()});
            options.includePaths.push_back(std::string(ShaderPath + "\\"));

            RHI::ShaderCompileResult spirvResult = SPIRVcompiler->SPIRVCompileFromFile(psshaderPath, options);
            RHI::ShaderCompileResult hlslResult = HLSLCompiler->HLSLCompileFromFile(psshaderPath, options);

            if (!spirvResult.success || !hlslResult.success) {
                std::cerr << "\n❌ Compilation failed!" << std::endl;
                std::cerr << "Error: " << spirvResult.errorMessage + " or " + hlslResult.errorMessage << std::endl;
                return 1;
            }
            // 打印警告信息 之后继续执行
            if (!spirvResult.warningMessage.empty() || !hlslResult.warningMessage.empty()) {
                std::cout << "Warnings: " << (spirvResult.warningMessage.empty() ? "" : spirvResult.warningMessage) + 
                            " or " + (hlslResult.warningMessage.empty() ? "" : hlslResult.warningMessage) << std::endl;
            }

            if (spirvResult.success) {
                std::cout << "SPIR-V compilation successfully!" << std::endl;
            }
            if (hlslResult.success) {
                std::cout << "HLSL compilation successfully!" << std::endl;
            }

            std::cout << "\nExtracting reflection information..." << std::endl;
            RHI::SPIRVReflection reflection = SPIRVreflector->ExtractReflection(spirvResult.byteCode);
            PrintReflection(reflection);

            if (vertexShader && pixelShader && computeShader)
            {
                std::cout << "VertexShader, PixelShader, ComputeShader compiled successfully!" << std::endl;

                // 创建根签名：包含一个 DescriptorTable + 一个 Root CBV
                RHI::RootSignatureDesc rootDesc;

                // ===== 参数 0：描述符表（含 2 个 Range）=====
                RHI::DescriptorRangeDesc ranges[1];

                // Range 0：CBV × 2，从 b0 开始
                ranges[0].RangeType      = RHI::DescriptorRangeType::CBV;
                ranges[0].NumDescriptors = 2;
                ranges[0].ShaderRegister = 0;
                ranges[0].RegisterSpace  = 0;

                RHI::RootParameterDesc tableParam = {};
                tableParam.Type                            = RHI::RootParameterType::DescriptorTable;
                tableParam.Visibility                      = RHI::ShaderVisibility::All;
                tableParam.DescriptorTable.NumDescriptorRanges = 1;
                tableParam.DescriptorTable.pDescriptorRanges   = ranges;

                // ===== 参数 1：Root CBV，绑定到 b2 =====
                RHI::RootParameterDesc cbvParam = {};
                cbvParam.Type                         = RHI::RootParameterType::CBV;
                cbvParam.Visibility                   = RHI::ShaderVisibility::All;
                cbvParam.Descriptor.ShaderRegister    = 2;  // b2（b0~b1 已被描述符表中的 CBV Range 占用）
                cbvParam.Descriptor.RegisterSpace     = 0;

                // ===== 参数 2：Root Constants，1 个 32 位值，绑定到 b3 =====
                RHI::RootParameterDesc constParam = {};
                constParam.Type                       = RHI::RootParameterType::Constants;
                constParam.Visibility                 = RHI::ShaderVisibility::Pixel;
                constParam.Constants.ShaderRegister   = 3;  // b3（b0~b2 已被前面的 CBV Range + Root CBV 占用）
                constParam.Constants.RegisterSpace    = 0;
                constParam.Constants.Num32BitValues   = 1;  // 1 个 float

                // ===== 组装根签名 =====
                rootDesc.Parameters.push_back(tableParam);
                rootDesc.Parameters.push_back(cbvParam);
                rootDesc.Parameters.push_back(constParam);
                rootDesc.Flags = RHI::RootSignatureFlags::AllowInputAssemblerInputLayout;
                
                RHI::RootSignatureDesc rootUavDesc;
                // ===== 参数 0：描述符表（含 2 个 Range）=====
                RHI::DescriptorRangeDesc uavranges[1];

                // Range 0：UAV × 1，从 t0 开始
                uavranges[0].RangeType      = RHI::DescriptorRangeType::UAV;
                uavranges[0].NumDescriptors = 1;
                uavranges[0].ShaderRegister = 0;
                uavranges[0].RegisterSpace  = 0;
                // OffsetInDescriptorsFromTableStart 默认为 ~0u (APPEND)

                RHI::RootParameterDesc uavtableParam = {};
                uavtableParam.Type                            = RHI::RootParameterType::DescriptorTable;
                uavtableParam.Visibility                      = RHI::ShaderVisibility::Compute;
                uavtableParam.DescriptorTable.NumDescriptorRanges = 1;
                uavtableParam.DescriptorTable.pDescriptorRanges   = uavranges;
                rootUavDesc.Parameters.push_back(uavtableParam);

                // 创建根签名
                auto rootSignature = device->CreateRootSignature(rootDesc);
                auto rootUAVSignature = device->CreateRootSignature(rootUavDesc);
                if (rootSignature && rootSignature->IsValid() && rootUAVSignature && rootUAVSignature->IsValid())
                {
                    std::cout << "RootSignature created successfully!" << std::endl;
                    
                    // 定义输入布局描述
                    RHI::InputElementDesc inputLayout[] = 
                    {
                        { "POSITION", 0, RHI::RHITextureFormat::R32G32B32_FLOAT, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT,
                            RHI::InputClassification::PerVertexData, 0 },
                            { "COLOR", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT,
                            RHI::InputClassification::PerVertexData, 0 },
                    };

                    RHI::GraphicsPipelineStateDesc graphicsDesc = {};
                    graphicsDesc.pRootSignature = rootSignature.get();
                    graphicsDesc.pInputElementDesc = inputLayout;
                    graphicsDesc.NumInputElements = sizeof(inputLayout) / sizeof(inputLayout[0]);
                    graphicsDesc.pVertexShader = vertexShader.get();
                    graphicsDesc.pPixelShader = pixelShader.get();
                    graphicsDesc.NumRenderTargets = 1;
                    graphicsDesc.RenderTargetFormats[0] = RHI::RHITextureFormat::R8G8B8A8_UNORM;
                    graphicsDesc.DepthStencilFormat = RHI::RHITextureFormat::Unknown;       // 绘制三角形不使用深度测试，所以这里设置为 Unknown

                    RHI::ComputePipelineStateDesc computeDesc = {};
                    computeDesc.pRootSignature = rootUAVSignature.get();
                    computeDesc.pComputeShader = computeShader.get();

                    auto computePSO = device->CreateComputePipelineState(computeDesc);
                    auto graphicsPSO = device->CreateGraphicsPipelineState(graphicsDesc);

                    if (graphicsPSO && graphicsPSO->IsValid())
                    {
                        std::cout << "GraphicsPipelineState created successfully!" << std::endl;

                        if (computePSO && computePSO->IsValid())
                        {
                            std::cout << "ComputePipelineState created successfully!" << std::endl;

                            auto cmdAllocator = device->CreateCommandAllocator(RHI::RHICmdType::Graphics);
                            auto cmdList = device->CreateCommandList(cmdAllocator);

                            if(cmdList && cmdList.get() != nullptr){
                                std::cout << "CommandList created successfully!" << std::endl;

                                // 获取描述符堆
                                auto descriptorHeap = device->GetDescriptorHeap(RHI::RHIDescriptorHeapType::Standard);
                                auto descriptorSamplerHeap = device->GetDescriptorHeap(RHI::RHIDescriptorHeapType::Sampler);

                                std::cout << "DescriptorHeaps Get successfully!" << std::endl;

                                MSG msg = {};
                                while (GetMessage(&msg, nullptr, 0, 0))
                                {
                                    if (msg.message == WM_QUIT){
                                        break;
                                    }
                                    TranslateMessage(&msg);
                                    DispatchMessage(&msg);
                                    
                                    // 开启帧
                                    device->GetCommandQueue(RHI::RHICmdType::Graphics)->BeginFrame();
                                    // 开始记录命令
                                    cmdList->BeginRecording();
                                    uint32_t currentIndex = swapChain->GetFrameIndex();
                                    RHI::RHITexture* BackBufferTexture = swapChain->GetBackBuffer(currentIndex);

                                    RHI::BarrierDesc barrier = {};
                                    barrier.Type                        = RHI::ResourceBarrierType::Transition;
                                    barrier.ResourceType                = RHI::BarrierResourceType::Texture;
                                    barrier.Flags                       = RHI::ResourceBarrierFlags::None;
                                    barrier.Transition.pResource        = BackBufferTexture;
                                    barrier.Transition.Subresource      = 0;
                                    barrier.Transition.StateBefore      = RHI::RHIResourceState::Present;
                                    barrier.Transition.StateAfter       = RHI::RHIResourceState::RenderTarget;
                                    
                                    cmdList->ResourceBarrier(1, &barrier);
                                    // 设置渲染目标视图
                                    RHI::RHIRenderTargetView* pRTV = swapChain->GetRenderTargetView(currentIndex);
                                    cmdList->OMSetRenderTargets(1, &pRTV, false, nullptr);
                                    // 清除渲染目标视图
                                    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
                                    cmdList->ClearRenderTargetView(pRTV, clearColor);

                                    cmdList->SetGraphicsRootSignature(rootSignature.get());
                                    cmdList->SetPipelineState(graphicsPSO.get(), RHI::PipelineStateType::Graphics);
                                    
                                    RHI::RHIDescriptorHeap* heaps[] = {descriptorHeap, descriptorSamplerHeap};
                                    cmdList->SetDescriptorHeaps(2, heaps);

                                    float floatValue = 0.5f; uint32_t intValue;
                                    memcpy(&intValue, &floatValue, sizeof(float));
                                    cmdList->SetGraphicsRootDescriptorTable(0, descriptorHeap, 0);
                                    cmdList->SetGraphicsRootConstantBufferView(1, cbvView->GetGPUVirtualAddress());
                                    cmdList->SetGraphicsRoot32BitConstant(2, intValue, 0);
                                    
                                    // 设置拓扑类型
                                    cmdList->IASetPrimitiveTopology(RHI::RHIPrimitiveTopology::TriangleList);
                                    // 设置顶点缓冲区
                                    RHI::RHIVertexBuffer* pRawBuffers[] = {vertexBuffer.get()};
                                    cmdList->IASetVertexBuffers(0, 1, pRawBuffers,0);

                                    uint32_t width = swapChain->GetWidth();
                                    uint32_t height = swapChain->GetHeight();

                                    RHI::RHIViewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
                                    cmdList->RSSetViewports(1, &viewport);

                                    RHI::RHIRect scissorRect = {0, 0, (float)width, (float)height};
                                    cmdList->RSSetScissorRects(1, &scissorRect);

                                    // 绘制三角形
                                    cmdList->Draw(3, 0);

                                    RHI::BarrierDesc barrierToPresent = {};
                                    barrierToPresent.Type                        = RHI::ResourceBarrierType::Transition;
                                    barrierToPresent.ResourceType                = RHI::BarrierResourceType::Texture;
                                    barrierToPresent.Flags                       = RHI::ResourceBarrierFlags::None;
                                    barrierToPresent.Transition.pResource        = BackBufferTexture;
                                    barrierToPresent.Transition.Subresource      = 0;
                                    barrierToPresent.Transition.StateBefore      = RHI::RHIResourceState::RenderTarget;
                                    barrierToPresent.Transition.StateAfter       = RHI::RHIResourceState::Present;
                                    cmdList->ResourceBarrier(1, &barrierToPresent);
                                    cmdList->EndRecording();
                                    
                                    // run command list
                                    device->GetCommandQueue(RHI::RHICmdType::Graphics)->ExecuteCommandLists({cmdList});
                                    swapChain->Present(1, 0);

                                    // 结束帧
                                    device->GetCommandQueue(RHI::RHICmdType::Graphics)->EndFrame();
                                    device->GetCommandQueue(RHI::RHICmdType::Graphics)->WaitForGPU();
                                    
                                    if (ExeSetSize) {
                                        ExeSetSize = false;
                                        swapChain->Resize(x, y);
                                    }
                                }
                            }else{
                                std::cout << "Failed to create CommandList!" << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << "Failed to create ComputePipelineState!" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Failed to create GraphicsPipelineState!" << std::endl;
                    }
                }
                else
                {
                    std::cout << "Failed to create RootSignature!" << std::endl;
                }
            }else{
                std::cout << "Failed to compile VertexShader!" << std::endl;
            }
        }
        else
        {
            std::cout << "Failed to create SwapChain!" << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to initialize Device!" << std::endl;
    }
    
    compilerContextController->ShutdownCompilerContext();
    device.reset(); 
    
    Core::SubsystemControl::Uninstall();
    return 0;
}
