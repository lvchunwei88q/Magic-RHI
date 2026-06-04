#include <CoreLogCapture/CoreLogCapture.h>
#include <Subsystem/Subsystem.h>
#include <iostream>
#include <windows.h>
#include <cstring>

#include <Core.h>
#include <RHI.hpp>
#include <IO.h>

#define PI 3.1415926f

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

#include <random>
#include <chrono>

struct Vertex
{
    float Position[3];  // XYZ
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
        
        // 随机颜色
        v.Color[0] = distColor(rng);
        v.Color[1] = distColor(rng);
        v.Color[2] = distColor(rng);
        v.Color[3] = 1.0f;
        
        vertices.push_back(v);
    }
    
    return vertices;
}

int main(int argc, char* argv[])
{
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;

    RHI::RHIType type = RHI::GetBestAvailableRHI();
    
    if (argc > 1 && strcmp(argv[1], "dx12") == 0) {
        type = RHI::RHIType::DirectX12;
        std::cout << "Using DirectX12" << std::endl;
    } else {
        type = RHI::RHIType::DirectX11;
        std::cout << "Using DirectX11" << std::endl;
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
        type == RHI::RHIType::DirectX11 ? L"RHI DirectX11 Window" : L"RHI DirectX12 Window",
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

    RHI::IRHILoader* loader = RHI::GetLoader();
    loader->Load(type);
    std::cout << "Is Multi-Threading Supported: " << (RHI::IsMultiThreadingSupported(loader->GetRHIType()) ? "Yes" : "No") << std::endl;
    
    auto device = loader->CreateDevice();
    if (device && device->Initialize())
    {
        std::cout << "Feature Level: " << GetFeatureLevelName(device->GetFeatureLevel()) << std::endl;
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
            
            // 现在测试创建采样器
            RHI::SamplerStateDesc desc{};
            desc.Filter = RHI::SamplerFilter::Trilinear;
            desc.AddressU = RHI::SamplerAddressMode::Clamp;
            std::shared_ptr<RHI::RHISamplerState> sampler = device->CreateSamplerState(desc);
            device->DeleteSamplerState(sampler);
            std::shared_ptr<RHI::RHISamplerState> sampler1 = device->CreateSamplerState(desc);
            std::shared_ptr<RHI::RHISamplerState> sampler2 = device->CreateSamplerState(desc);
            if (sampler2 && sampler1)
            {
                std::cout << "SamplerState created successfully!" << std::endl;

                std::vector<Vertex> vertices = GenerateRandomVertices(2 * 1024 * 1024);  // 2 MB
                std::vector<Vertex> cvertices = GenerateRandomVertices(2 * 1024);  // 2 KB
                std::cout << "Vertices created successfully! Vertex Count: " << vertices.size() << std::endl;

                RHI::BufferDesc desc;
                desc.SizeInBytes =  sizeof(Vertex) * vertices.size(); // 总字节数
                desc.Stride       = sizeof(Vertex);                    // 每个顶点的大小
                desc.InitialData  = vertices.data();                   // 指向初始数据的指针
                desc.HeapType     = RHI::BufferHeapType::Default;
                desc.BindFlags    = RHI::BufferBindFlag::VertexBuffer;
                // ========== 3. 调用 CreateBuffer 创建顶点缓冲 ==========
                std::shared_ptr<RHI::RHIVertexBuffer> vertexBuffer = device->CreateBuffer(desc);
                //device->DeleteBuffer(vertexBuffer);
                std::cout << "VertexBuffer created successfully!" << std::endl;
                
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
                
                std::shared_ptr<RHI::RHIConstantBuffer> constantBuffer2 = CreateBuffer(RHI::BufferHeapType::Upload,RHI::BufferBindFlag::ShaderResource,RHI::DescriptorRangeType::SRV);
                std::cout << "ShaderResource created successfully!" << std::endl;
                std::cout << "ShaderResource Handle: " << constantBuffer2->GetBindlessHandle().GetIndex() << std::endl;
                
                std::cout << "Shader compilation..." << std::endl;
                // 从文件编译顶点着色器
                std::string exePath = IO::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
                std::string vsshaderPath = exePath + "\\..\\..\\Test\\testVS.hlsl"; // 你知道的这只是一个测试示例
                std::string psshaderPath = exePath + "\\..\\..\\Test\\testPS.hlsl";
                std::string csshaderPath = exePath + "\\..\\..\\Test\\testCS.hlsl";
                RHI::ShaderCompileDesc vsDesc{};
                vsDesc.Type = RHI::ShaderType::Vertex;
                vsDesc.EnableDebugInfo = true;
                vsDesc.FilePath = vsshaderPath.c_str();

                RHI::ShaderCompileDesc psDesc{};
                psDesc.Type = RHI::ShaderType::Pixel;
                psDesc.EnableDebugInfo = true;
                psDesc.FilePath = psshaderPath.c_str();

                RHI::ShaderCompileDesc csDesc{};
                csDesc.Type = RHI::ShaderType::Compute;
                csDesc.EnableDebugInfo = true;
                csDesc.FilePath = csshaderPath.c_str();

                auto vertexShader = device->CompileVertexShader(vsDesc);
                auto pixelShader = device->CompilePixelShader(psDesc);
                auto computeShader = device->CompileComputeShader(csDesc);

                if (vertexShader && pixelShader && computeShader)
                {
                    std::cout << "VertexShader, PixelShader, ComputeShader compiled successfully!" << std::endl;

                    // 创建根签名：包含一个 DescriptorTable + 一个 Root CBV
                    RHI::RootSignatureDesc rootDesc;

                    // ===== 参数 0：描述符表（含 2 个 Range）=====
                    RHI::DescriptorRangeDesc ranges[2];

                    // Range 0：SRV × 4，从 t0 开始
                    ranges[0].RangeType      = RHI::DescriptorRangeType::SRV;
                    ranges[0].NumDescriptors = 4;
                    ranges[0].ShaderRegister = 0;
                    ranges[0].RegisterSpace  = 0;
                    // OffsetInDescriptorsFromTableStart 默认为 ~0u (APPEND)

                    // Range 1：CBV × 2，从 b0 开始
                    ranges[1].RangeType      = RHI::DescriptorRangeType::CBV;
                    ranges[1].NumDescriptors = 2;
                    ranges[1].ShaderRegister = 0;
                    ranges[1].RegisterSpace  = 0;

                    RHI::RootParameterDesc tableParam = {};
                    tableParam.Type                            = RHI::RootParameterType::DescriptorTable;
                    tableParam.Visibility                      = RHI::ShaderVisibility::VertexPixel;
                    tableParam.DescriptorTable.NumDescriptorRanges = 2;
                    tableParam.DescriptorTable.pDescriptorRanges   = ranges;

                    // ===== 参数 1：Root CBV，绑定到 b2 =====
                    RHI::RootParameterDesc cbvParam = {};
                    cbvParam.Type                         = RHI::RootParameterType::CBV;
                    cbvParam.Visibility                   = RHI::ShaderVisibility::VertexPixel;
                    cbvParam.Descriptor.ShaderRegister    = 2;  // b2（b0~b1 已被描述符表中的 CBV Range 占用）
                    cbvParam.Descriptor.RegisterSpace     = 0;

                    // ===== 参数 2：Root Constants，4 个 32 位值，绑定到 b3 =====
                    RHI::RootParameterDesc constParam = {};
                    constParam.Type                       = RHI::RootParameterType::Constants;
                    constParam.Visibility                 = RHI::ShaderVisibility::VertexPixel;
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
                    uavtableParam.Visibility                      = RHI::ShaderVisibility::ComputeBit;
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
                            { "POSITION", 0, RHI::RHITextureFormat::R16_UNORM, 0, RHI::InputElementDesc::APPEND_ALIGNED_ELEMENT,
                                RHI::InputClassification::PerVertexData, 0 },
                        };

                        RHI::GraphicsPipelineStateDesc graphicsDesc = {};
                        graphicsDesc.pRootSignature = rootSignature.get();
                        graphicsDesc.pInputElementDesc = inputLayout;
                        graphicsDesc.NumInputElements = sizeof(inputLayout) / sizeof(inputLayout[0]);
                        graphicsDesc.pVertexShader = vertexShader.get();
                        graphicsDesc.pPixelShader = pixelShader.get();
                        graphicsDesc.NumRenderTargets = 1;
                        graphicsDesc.RenderTargetFormats[0] = RHI::RHITextureFormat::R32G32B32A32_FLOAT;
                        graphicsDesc.DepthStencilFormat = RHI::RHITextureFormat::D32_FLOAT;

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
                                        
                                        cmdList->BeginRecording();

                                        cmdList->SetGraphicsRootSignature(rootSignature.get());
                                        cmdList->SetPipelineState(graphicsPSO.get(), RHI::PipelineStateType::Graphics);
                                        
                                        RHI::RHIDescriptorHeap* heaps[] = {descriptorHeap, descriptorSamplerHeap};
                                        cmdList->SetDescriptorHeaps(2, heaps);

                                        cmdList->SetGraphicsRootDescriptorTable(0, descriptorHeap, 0);
                                        cmdList->SetGraphicsRootConstantBufferView(1, constantBuffer1->GetGPUVirtualAddress());
                                        cmdList->SetGraphicsRoot32BitConstant(2, PI, 0);
                                        
                                        // 设置拓扑类型
                                        cmdList->IASetPrimitiveTopology(RHI::RHIPrimitiveTopology::TriangleList);
                                        cmdList->EndRecording();
                                        
                                        // run command list
                                        device->GetCommandQueue(RHI::RHICmdType::Graphics)->ExecuteCommandLists({cmdList});
                                        //swapChain->Present();
                                        device->GetCommandQueue(RHI::RHICmdType::Graphics)->WaitForGPU();
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
                std::cout << "Failed to create SamplerState!" << std::endl;
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
    
    device.reset(); 
    
    Core::SubsystemControl::Uninstall();
    return 0;
}
