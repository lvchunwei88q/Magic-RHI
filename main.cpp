#include <CoreLogCapture/CoreLogCapture.h>
#include <Subsystem/Subsystem.h>
#include <iostream>
#include <windows.h>
#include <cstring>

#include <Core.h>
#include <RHI.hpp>
#include <AbsolutePath.h>
#include <Converter.h>


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

std::vector<Vertex> GenerateRandomVertices(size_t targetSizeMB)
{
    constexpr size_t vertexSize = sizeof(Vertex);  // 28 bytes
    size_t targetVertexCount = (targetSizeMB * 1024 * 1024) / vertexSize;
    
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

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

std::vector<std::string> GetLoadedDLLs()
{
    std::vector<std::string> dlls;
    
    HANDLE hProcess = GetCurrentProcess();
    HMODULE modules[1024];
    DWORD needed;
    
    if (EnumProcessModules(hProcess, modules, sizeof(modules), &needed))
    {
        DWORD count = needed / sizeof(HMODULE);
        char filename[MAX_PATH];
        
        for (DWORD i = 0; i < count; i++)
        {
            if (GetModuleFileNameExA(hProcess, modules[i], filename, sizeof(filename)))
            {
                // 只提取文件名（不含路径）
                std::string fullPath = filename;
                size_t pos = fullPath.find_last_of("\\/");
                std::string dllName = (pos != std::string::npos) ? fullPath.substr(pos + 1) : fullPath;
                dlls.push_back(dllName);
            }
        }
    }
    
    return dlls;
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

    // 打印已加载的DLL
    std::vector<std::string> loadedDLLs = GetLoadedDLLs();
    for (const std::string& dll : loadedDLLs) {
        std::cout << "Loaded DLL: " << dll << std::endl;
    }
    
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

                std::vector<Vertex> vertices = GenerateRandomVertices(2);  // 2 MB
                std::cout << "Vertices created successfully! Vertex Count: " << vertices.size() << std::endl;

                uint64_t bufferSize = sizeof(Vertex) * vertices.size();
                std::cout << "VertexBuffer Size: " << bufferSize << std::endl;

                RHI::BufferDesc desc;
                desc.SizeInBytes =  bufferSize; // 总字节数
                desc.Stride       = sizeof(Vertex);                    // 每个顶点的大小
                desc.InitialData  = vertices.data();                   // 指向初始数据的指针
                desc.HeapType     = RHI::BufferHeapType::Upload;
                desc.BindFlags    = RHI::BufferBindFlag::VertexBuffer;
                // ========== 3. 调用 CreateBuffer 创建顶点缓冲 ==========
                std::shared_ptr<RHI::RHIVertexBuffer> vertexBuffer = device->CreateBuffer(desc);
                //device->DeleteBuffer(vertexBuffer);
                std::cout << "VertexBuffer created successfully!" << std::endl;

                std::cout << "Shader compilation..." << std::endl;
                // 从文件编译顶点着色器
                std::string exePath = IO::Converter::ToNarrowString(IO::AbsolutePath::Get().GetExecutableDirectory());
                std::string shaderPath = exePath + "\\..\\..\\Test\\test.hlsl"; // 你知道的这只是一个测试示例
                RHI::ShaderCompileDesc vsDesc{};
                vsDesc.Type = RHI::ShaderType::Vertex;
                vsDesc.EnableDebugInfo = true;
                vsDesc.FilePath = shaderPath.c_str();

                auto vertexShader = device->CompileVertexShader(vsDesc);

                if (vertexShader)
                {
                    std::cout << "VertexShader compiled successfully!" << std::endl;
                    // 创建根签名
                    RHI::RootSignatureDesc rootDesc;
                    rootDesc.Parameters.push_back({RHI::RootParameterType::CBV, 0, 0});
                    rootDesc.Flags = RHI::RootSignatureFlags::AllowInputAssemblerInputLayout;

                    auto rootSignature = device->CreateRootSignature(rootDesc);
                    if (rootSignature && rootSignature->IsValid()) {
                        // 使用根签名...
                        std::cout << "RootSignature created successfully!" << std::endl;
                        // 删除根签名
                        device->DeleteRootSignature(rootSignature);

                        MSG msg = {};
                        while (GetMessage(&msg, nullptr, 0, 0))
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
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
    
    Core::SubsystemControl::Uninstall();
    return 0;
}
