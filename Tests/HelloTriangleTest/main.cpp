#include <Subsystem/Subsystem.h>
#include <iostream>
#include <windows.h>
#include <cstring>

#include <CoreMinimal.h>
#include <RHI.hpp>
#include <IO.h>

#include "../Common/Test.h"

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

struct Vertex
{
    float Position[4];  // XYZW
    float Color[4];     // RGBA
};

std::wstring GetRHIWindowName(RHI::RHIType type)
{
    if (EnumHasAnyFlags(type, RHI::RHIType::None)) {
        return L"RHI None Window";
    }else if (EnumHasAnyFlags(type, RHI::RHIType::D3D11)) {
        return L"RHI D3D11 Window";
    }else if (EnumHasAnyFlags(type, RHI::RHIType::D3D12)) {
        return L"RHI D3D12 Window";
    }else if (EnumHasAnyFlags(type, RHI::RHIType::VulKan)) {
        return L"RHI Vulkan Window";
    }else if (EnumHasAnyFlags(type, RHI::RHIType::Unknown)) {
        return L"RHI Unknown Window";
    }else{
        return L"RHI Unknown Window";
    }
}

// 测试代码，不作为开发的一部分用完即弃
// 这个测试代码主要用于验证RHI模块的功能，包括设备初始化、交换链
int main(int argc, char* argv[])
{
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;

    RHI::RHIType type = RHI::GetBestAvailableRHI();
    std::string SupportTable = RHITypeToString(type);
    type = RHI::RHIType::None; 

    std::cout << "Best Available RHI: " << SupportTable << std::endl;
    std::cout << "Select RHI Type: ";
    std::cin >> SupportTable;
    
    if (SupportTable == "dx12") {
        type |= RHI::RHIType::D3D12;
    } else if (SupportTable == "vk") {
        type |= RHI::RHIType::VulKan;
    }else if (SupportTable == "dx11") {
        type |= RHI::RHIType::D3D11;
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

    std::wstring windowName = GetRHIWindowName(type);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        windowName.c_str(),
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
    // 加载RHI模块
    if(!loader->Load(type)){
        return 1;
    }

    std::unique_ptr<RHIShaderCompiler> compilerContextController = RHIModule::GetCompilerContextController();
    RHIShaderCompiler* Compiler = RHIModule::GetCompilerPipeline();

    bool isInitialized = compilerContextController->InitializeCompilerThreadContext();
    if (!isInitialized) {
        std::cerr << "Failed to initialize shader compiler context!" << std::endl;
        return 1;
    }

    if (!Compiler) {
        std::cerr << "Failed to get compiler pipeline!" << std::endl;
        return 1;
    }

    auto device = loader->CreateDevice();
    if (device && device->Initialize() && device->IsValid())
    {   
        std::cout << "Feature Level: " << GetFeatureLevelName(device->GetFeatureLevel()) << std::endl;
        std::cout << "Shader Model Version: " << ShaderModelToString(device->GetShaderModelVersion()) << std::endl;
        std::wcout << L"Device initialized successfully: " << device->GetAdapterName() << std::endl;
        
        RHI::SwapChainDesc swapChainDesc = {};
        swapChainDesc.WindowHandleRef = hwnd;
        swapChainDesc.Width = 800;
        swapChainDesc.Height = 600;
        swapChainDesc.VSync = false;
        
        auto swapChain = loader->CreateSwapChain();
        if (swapChain && swapChain->Initialize(device.get(), swapChainDesc) && swapChain->IsValid())
        {
            std::cout << "SwapChain created successfully!" << std::endl;

            uint64_t frequency = 0;
            device->GetCommandQueue(RHI::RHICmdType::Graphics)->GetTimestampFrequency(&frequency);
            std::cout << "Timestamp Frequency: " << frequency << std::endl;

            Vertex tvertices[] = {
                // 位置 (X, Y, Z, W)              // 颜色 (R, G, B, A)
                { {  0.0f,  0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // 顶点0: 顶部 (纯红)
                { {  0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // 顶点1: 右下 (纯绿)
                { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }   // 顶点2: 左下 (纯蓝)
            };

            // 定义输入布局描述
            std::vector<RHI::InputElementDesc> inputLayout = 
            {
                { "POSITION", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT,
                    RHI::InputClassification::PerVertexData, 0 },
                { "COLOR", 0, RHI::RHITextureFormat::R32G32B32A32_FLOAT, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT,
                    RHI::InputClassification::PerVertexData, 0 },
            };

            RHI::BufferDesc vbdesc;
            vbdesc.SizeInBytes = sizeof(Vertex) * 3;
            vbdesc.Stride = sizeof(Vertex);
            vbdesc.InitialData = tvertices;
            vbdesc.HeapType = RHI::BufferHeapType::Default;
            vbdesc.BindFlags = RHI::BufferBindFlag::VertexBuffer;
            std::shared_ptr<RHI::RHIVertexBuffer> vertexBuffer = device->CreateBuffer(vbdesc);
            std::cout << "VertexBuffer created successfully!" << std::endl;
            
            std::cout << "Shader compilation..." << std::endl;
            // 从文件编译顶点着色器
            std::string exePath = IO::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
            std::string ShaderPath = exePath + "\\..\\..\\Tests\\HelloTriangleTest";
            std::string vsshaderPath = ShaderPath + "\\testVS.hlsl"; // 你知道的这只是一个测试示例
            std::string psshaderPath = ShaderPath + "\\testPS.hlsl";
            RHI::ShaderCompileOptions vsOption{};
            vsOption.targetProfile = "vs_6_0";
            vsOption.debugInfo = true;
            vsOption.optimize = true;
            vsOption.entryPoint = "main";
            vsOption.includePaths.push_back(std::string(ShaderPath + "\\"));

            RHI::ShaderCompileOptions psOption{};
            psOption.targetProfile = "ps_6_0";
            psOption.debugInfo = true;
            psOption.optimize = true;
            psOption.entryPoint = "main";
            psOption.includePaths.push_back(std::string(ShaderPath + "\\"));

            Compiler->BeginCompiler();
            // VS
            RHI::ShaderCompileSource Source{};
            Source.sourceType = RHI::ShaderCompileSource::SourceType::SourcePath;
            Source.sourceDescription = vsshaderPath;
            Source.postProcessArgs = std::make_shared<RHI::VertexShaderPostProcessArgs>(inputLayout);
            RHI::ShaderCompileResult vsResult = Compiler->Compile(vsOption,Source);
            RHI::CreateShaderDesc vsDesc = Compiler->CreateShaderDescription();
            // PS
            Source.sourceDescription = psshaderPath;
            // PixelShader 不需要 post-process
            Source.postProcessArgs = nullptr;
            RHI::ShaderCompileResult psResult = Compiler->Compile(psOption,Source);
            RHI::CreateShaderDesc psDesc = Compiler->CreateShaderDescription();
            // psreflection
            RHI::SPIRVReflection psreflection = Compiler->Reflection();

            if (!vsResult.success || !psResult.success) {
                std::cerr << "\n❌ Compilation failed!" << std::endl;
                std::cerr << "Error: " << vsResult.errorMessage + " or " + psResult.errorMessage << std::endl;
                return 1;
            }
            // 打印警告信息 之后继续执行
            if (!vsResult.warningMessage.empty() || !psResult.warningMessage.empty()) {
                std::cout << "Warnings: " << (vsResult.warningMessage.empty() ? "" : vsResult.warningMessage) + 
                            " or " + (psResult.warningMessage.empty() ? "" : psResult.warningMessage) << std::endl;
            }

            Compiler->EndCompiler();

            // 创建着色器
            std::shared_ptr<RHI::RHIVertexShader> vertexShader = device->CreateVertexShader(vsDesc);
            std::shared_ptr<RHI::RHIPixelShader> pixelShader = device->CreatePixelShader(psDesc);

            if (vertexShader && pixelShader)
            {
                std::cout << "VertexShader, PixelShader compiled successfully!" << std::endl;

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

                // ===== 参数 2：Root Constants，1 个 32 位值，绑定到 b3 =====
                RHI::RootParameterDesc constParam = {};
                constParam.Type                       = RHI::RootParameterType::Constants;
                constParam.Visibility                 = RHI::ShaderVisibility::Pixel;
                constParam.Constants.ShaderRegister   = 2;  // b2（b0~b1 已被前面的 CBV Range
                constParam.Constants.RegisterSpace    = 0;
                constParam.Constants.Num32BitValues   = 1;  // 1 个 float

                // ===== 组装根签名 =====
                rootDesc.Parameters.push_back(tableParam);
                rootDesc.Parameters.push_back(constParam);
                rootDesc.Flags = RHI::RootSignatureFlags::AllowInputAssemblerInputLayout;

                // 创建根签名
                auto rootSignature = device->CreateRootSignature(rootDesc);
                if (rootSignature && rootSignature->IsValid())
                {
                    std::cout << "RootSignature created successfully!" << std::endl;

                    RHI::GraphicsPipelineStateDesc graphicsDesc = {};
                    graphicsDesc.pRootSignature = rootSignature.get();
                    graphicsDesc.pInputElementDesc = inputLayout.data();
                    graphicsDesc.NumInputElements = inputLayout.size();
                    graphicsDesc.pVertexShader = vertexShader.get();
                    graphicsDesc.pPixelShader = pixelShader.get();
                    graphicsDesc.NumRenderTargets = 1;
                    graphicsDesc.RenderTargetFormats[0] = RHI::RHITextureFormat::R8G8B8A8_UNORM;
                    graphicsDesc.DepthStencilFormat = RHI::RHITextureFormat::Unknown;       // 绘制三角形不使用深度测试，所以这里设置为 Unknown

                    auto graphicsPSO = device->CreateGraphicsPipelineState(graphicsDesc);

                    if (graphicsPSO && graphicsPSO->IsValid())
                    {
                        std::cout << "GraphicsPipelineState created successfully!" << std::endl;

                        auto cmdPool = device->CreateCommandPool(RHI::RHICmdType::Graphics);
                        auto cmdList = device->CreateCommandList(cmdPool);

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

                                device->GetCommandQueue(RHI::RHICmdType::Graphics)->ResetCommandPoolMemory(cmdPool.get());

                                // 开始记录命令列表
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
                                cmdList->SetGraphicsRoot32BitConstant(1, intValue, 0);
                                
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
    
    compilerContextController->ShutdownCompilerThreadContext();
    device.reset(); 
    
    // 注意所有的释放必须在这里之前完成否则将无法释放
    Core::SubsystemControl::Uninstall();
    return 0;
}
