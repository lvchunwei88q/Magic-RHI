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

std::wstring GetRHIWindowName(RHI::RHIType type)
{
    if (EnumHasAnyFlags(type, RHI::RHIType::None)) {
        return L"RHI None Window";
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

// tests function
void TestHelloTriangle()
{
    {
        struct Vertex
        {
            float Position[4];  // XYZW
            float Color[4];     // RGBA
        };

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
        }

        Core::SubsystemControl::Init();

        { // register core log capture
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
            0, CLASS_NAME, windowName.c_str(), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );
        
        // Use a test assertion instead of the original return 1;
        EXPECT_TRUE(hwnd != nullptr);

        ShowWindow(hwnd, SW_SHOW);
        
        std::cout << "Initializing Device..." << std::endl;

        RHIAPILoader* loader = RHIModule::GetRHILoader();
        
        // load RHI DLL and verify success
        ASSERT_TRUE(loader->Load(type));

        std::unique_ptr<RHIShaderCompiler> compilerContextController = RHIModule::GetCompilerContextController();
        RHIShaderCompiler* Compiler = RHIModule::GetCompilerPipeline();

        // Swap out the original `if (!isInitialized) return 1;`
        ASSERT_TRUE(compilerContextController->InitializeCompilerThreadContext());
        ASSERT_TRUE(Compiler != nullptr);

        auto device = loader->CreateDevice();
        ASSERT_TRUE(device && device->Initialize() && device->IsValid());
        
        std::cout << "Feature Level: " << GetFeatureLevelName(device->GetFeatureLevel()) << std::endl;
        std::cout << "Shader Model Version: " << ShaderModelToString(device->GetShaderModelVersion()) << std::endl;
        std::wcout << L"Device initialized successfully: " << device->GetAdapterName() << std::endl;
        
        RHI::SwapChainDesc swapChainDesc = {};
        swapChainDesc.WindowHandleRef = hwnd;
        swapChainDesc.Width = 800;
        swapChainDesc.Height = 600;
        swapChainDesc.VSync = false;
        
        auto swapChain = loader->CreateSwapChain();
        ASSERT_TRUE(swapChain && swapChain->Initialize(device.get(), swapChainDesc) && swapChain->IsValid());
        
        std::cout << "SwapChain created successfully!" << std::endl;

        uint64_t frequency = 0;
        device->GetFrameCommandQueue()->GetTimestampFrequency(&frequency);
        std::cout << "Timestamp Frequency: " << frequency << std::endl;

        Vertex tvertices[] = {
            { {  0.0f,  0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { {  0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

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
        
        ASSERT_TRUE(vertexBuffer != nullptr);
        std::cout << "VertexBuffer created successfully!" << std::endl;
        
        std::cout << "Shader compilation..." << std::endl;
        
        std::string exePath = IO::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
        std::string ShaderPath = exePath + "\\..\\..\\Tests\\HelloTriangleTest\\Shader";
        std::string vsshaderPath = ShaderPath + "\\testVS.hlsl"; 
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
        
        RHI::ShaderCompileSource Source{};
        Source.sourceType = RHI::ShaderCompileSource::SourceType::SourcePath;
        Source.sourceDescription = vsshaderPath;
        Source.postProcessArgs = std::make_shared<RHI::VertexShaderPostProcessArgs>(inputLayout);
        RHI::ShaderCompileResult vsResult = Compiler->Compile(vsOption,Source);
        RHI::CreateShaderDesc vsDesc = Compiler->CreateShaderDescription();
        
        Source.sourceDescription = psshaderPath;
        Source.postProcessArgs = nullptr;
        RHI::ShaderCompileResult psResult = Compiler->Compile(psOption,Source);
        RHI::CreateShaderDesc psDesc = Compiler->CreateShaderDescription();
        RHI::SPIRVReflection psreflection = Compiler->Reflection();

        if (!vsResult.success || !psResult.success) {
            std::cerr << "\n Compilation failed!" << std::endl;
            std::cerr << "Error: " << vsResult.errorMessage + " or " + psResult.errorMessage << std::endl;
            // If the compilation fails, use EXPECT_FALSE to let the test framework record the failure,
            // then return to prevent a crash.
            EXPECT_FALSE(!vsResult.success || !psResult.success);
            return; 
        }
        
        if (!vsResult.warningMessage.empty() || !psResult.warningMessage.empty()) {
            std::cout << "Warnings: " << (vsResult.warningMessage.empty() ? "" : vsResult.warningMessage) + 
                        " or " + (psResult.warningMessage.empty() ? "" : psResult.warningMessage) << std::endl;
        }

        Compiler->EndCompiler();

        std::shared_ptr<RHI::RHIVertexShader> vertexShader = device->CreateVertexShader(vsDesc);
        std::shared_ptr<RHI::RHIPixelShader> pixelShader = device->CreatePixelShader(psDesc);

        ASSERT_TRUE(vertexShader && pixelShader);
        std::cout << "VertexShader, PixelShader compiled successfully!" << std::endl;

        RHI::RootSignatureDesc rootDesc;
        RHI::DescriptorRangeDesc ranges[1];
        ranges[0].RangeType      = RHI::DescriptorRangeType::CBV;
        ranges[0].NumDescriptors = 2;
        ranges[0].ShaderRegister = 0;
        ranges[0].RegisterSpace  = 0;

        RHI::RootParameterDesc tableParam = {};
        tableParam.Type                            = RHI::RootParameterType::DescriptorTable;
        tableParam.Visibility                      = RHI::ShaderVisibility::All;
        tableParam.DescriptorTable.NumDescriptorRanges = 1;
        tableParam.DescriptorTable.pDescriptorRanges   = ranges;

        RHI::RootParameterDesc constParam = {};
        constParam.Type                       = RHI::RootParameterType::Constants;
        constParam.Visibility                 = RHI::ShaderVisibility::Pixel;
        constParam.Constants.ShaderRegister   = 2; 
        constParam.Constants.RegisterSpace    = 0;
        constParam.Constants.Num32BitValues   = 1; 

        rootDesc.Parameters.push_back(tableParam);
        rootDesc.Parameters.push_back(constParam);
        rootDesc.Flags = RHI::RootSignatureFlags::AllowInputAssemblerInputLayout;

        auto rootSignature = device->CreateRootSignature(rootDesc);
        ASSERT_TRUE(rootSignature && rootSignature->IsValid());
        std::cout << "RootSignature created successfully!" << std::endl;

        RHI::GraphicsPipelineStateDesc graphicsDesc = {};
        graphicsDesc.pRootSignature = rootSignature.get();
        graphicsDesc.pInputElementDesc = inputLayout.data();
        graphicsDesc.NumInputElements = inputLayout.size();
        graphicsDesc.pVertexShader = vertexShader.get();
        graphicsDesc.pPixelShader = pixelShader.get();
        graphicsDesc.NumRenderTargets = 1;
        graphicsDesc.RenderTargetFormats[0] = swapChain->GetRenderTargetFormat();
        graphicsDesc.DepthStencilFormat = RHI::RHITextureFormat::Unknown;       

        auto graphicsPSO = device->CreateGraphicsPipelineState(graphicsDesc);
        ASSERT_TRUE(graphicsPSO && graphicsPSO->IsValid());
        std::cout << "GraphicsPipelineState created successfully!" << std::endl;

        std::vector<std::shared_ptr<RHI::RHICommandPool>> cmdPools (RHI_MULTI_BUFFERING);
        std::vector<std::shared_ptr<RHI::RHICommandList>> cmdLists (RHI_MULTI_BUFFERING);

        for (size_t i = 0; i < RHI_MULTI_BUFFERING; ++i)
        {
            cmdPools[i] = device->CreateCommandPool (RHI::RHICmdType::Graphics);
            cmdLists[i] = device->CreateCommandList (cmdPools[i]);
            ASSERT_TRUE (cmdLists[i] && cmdLists[i].get () != nullptr);
        }

        std::cout << "CommandList created successfully!" << std::endl;

        auto descriptorHeap = device->GetDescriptorHeap(RHI::RHIDescriptorHeapType::Standard);
        auto descriptorSamplerHeap = device->GetDescriptorHeap(RHI::RHIDescriptorHeapType::Sampler);

        ASSERT_TRUE(descriptorHeap != nullptr && descriptorSamplerHeap != nullptr);
        std::cout << "DescriptorHeaps Get successfully!" << std::endl;

        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (msg.message == WM_QUIT){
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            device->GetFrameCommandQueue()->BeginFrame();
            uint32_t ImageIndex = swapChain->AcquireNextImage ();
            device->GetFrameCommandQueue()->ResetCommandPoolMemory(cmdPools[ImageIndex].get ());
            RHI::GraphicsQueueSwapChainInfo GQSCInfo = swapChain->GetGraphicsQueueSwapChainInfo();
            device->GetFrameCommandQueue()->SetGraphicsQueueSwapChainInfo(GQSCInfo);

            cmdLists[ImageIndex]->BeginRecording ();
            RHI::RHITexture* BackBufferTexture = swapChain->GetBackBuffer(ImageIndex);

            RHI::BarrierDesc barrier = {};
            barrier.Type                        = RHI::ResourceBarrierType::Transition;
            barrier.ResourceType                = RHI::BarrierResourceType::Texture;
            barrier.Flags                       = RHI::ResourceBarrierFlags::None;
            barrier.Transition.pResource        = BackBufferTexture;
            barrier.Transition.Subresource      = 0;
            barrier.Transition.StateBefore      = RHI::RHIResourceState::Present;
            barrier.Transition.StateAfter       = RHI::RHIResourceState::RenderTarget;
            
            cmdLists[ImageIndex]->ResourceBarrier(1, &barrier);
            
            RHI::RHIRenderTargetView* pRTV = swapChain->GetRenderTargetView(ImageIndex);
            cmdLists[ImageIndex]->OMSetRenderTargets(1, &pRTV, false, nullptr);
            
            float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            cmdLists[ImageIndex]->ClearRenderTargetView(pRTV, clearColor);

            cmdLists[ImageIndex]->SetGraphicsRootSignature(rootSignature.get());
            cmdLists[ImageIndex]->SetPipelineState(graphicsPSO.get(), RHI::PipelineStateType::Graphics);
            
            RHI::RHIDescriptorHeap* heaps[] = {descriptorHeap, descriptorSamplerHeap};
            cmdLists[ImageIndex]->SetDescriptorHeaps(2, heaps);

            float floatValue = 0.5f; uint32_t intValue;
            memcpy(&intValue, &floatValue, sizeof(float));
            cmdLists[ImageIndex]->SetGraphicsRootDescriptorTable(0, descriptorHeap, 0);
            cmdLists[ImageIndex]->SetGraphicsRoot32BitConstant(1, intValue, 0);
            
            cmdLists[ImageIndex]->IASetPrimitiveTopology(RHI::RHIPrimitiveTopology::TriangleList);
            
            RHI::RHIVertexBuffer* pRawBuffers[] = {vertexBuffer.get()};
            cmdLists[ImageIndex]->IASetVertexBuffers(0, 1, pRawBuffers,0);

            uint32_t width = swapChain->GetWidth();
            uint32_t height = swapChain->GetHeight();

            RHI::RHIViewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
            cmdLists[ImageIndex]->RSSetViewports(1, &viewport);

            RHI::RHIRect scissorRect = {0, 0, (float)width, (float)height};
            cmdLists[ImageIndex]->RSSetScissorRects(1, &scissorRect);

            cmdLists[ImageIndex]->Draw(3, 0);

            RHI::BarrierDesc barrierToPresent = {};
            barrierToPresent.Type                        = RHI::ResourceBarrierType::Transition;
            barrierToPresent.ResourceType                = RHI::BarrierResourceType::Texture;
            barrierToPresent.Flags                       = RHI::ResourceBarrierFlags::None;
            barrierToPresent.Transition.pResource        = BackBufferTexture;
            barrierToPresent.Transition.Subresource      = 0;
            barrierToPresent.Transition.StateBefore      = RHI::RHIResourceState::RenderTarget;
            barrierToPresent.Transition.StateAfter       = RHI::RHIResourceState::Present;
            cmdLists[ImageIndex]->ResourceBarrier(1, &barrierToPresent);
            cmdLists[ImageIndex]->EndRecording();
            
            device->GetFrameCommandQueue()->ExecuteCommandLists({ cmdLists[ImageIndex]});
            device->GetFrameCommandQueue()->Present();

            device->GetFrameCommandQueue()->EndFrame();
            
            if (ExeSetSize) {
                ExeSetSize = false;
                device->GetFrameCommandQueue()->WaitForGPU();
                swapChain->Resize(x, y);
            }
        }
        // ========== WaitForGPU ==========
        device->GetFrameCommandQueue ()->WaitForGPU ();
        // ========== cleanup compiler context ==========
        compilerContextController->ShutdownCompilerThreadContext();
    }
    // cleanup subsystem
    Core::SubsystemControl::Uninstall();
}

// Use the macro-defined main function of the test framework to drive test cases
TEST_MAIN(TestHelloTriangle)