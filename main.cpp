#include "Engine/Source/Core/Core/Public/Core.h"
#include <iostream>
#include <windows.h>
#include <cstring>

#include <Core.h>
#include <RHI.h>
#include <RHIResource.h>

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

int main(int argc, char* argv[])
{
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;

    RHI::RHIType type = RHI::RHIType::DirectX11;
    
    if (argc > 1 && strcmp(argv[1], "dx12") == 0) {
        type = RHI::RHIType::DirectX12;
        std::cout << "Using DirectX12" << std::endl;
    } else {
        std::cout << "Using DirectX11" << std::endl;
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
    
    auto device = RHI::Device::Create(type);
    if (device && device->Initialize())
    {
        std::wcout << L"Device initialized successfully: " << device->GetAdapterName() << std::endl;
        
        RHI::SwapChainDesc swapChainDesc = {};
        swapChainDesc.WindowHandle = hwnd;
        swapChainDesc.Width = 800;
        swapChainDesc.Height = 600;
        swapChainDesc.VSync = false;
        
        auto swapChain = RHI::SwapChain::Create(type);
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

                MSG msg = {};
                while (GetMessage(&msg, nullptr, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
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
    
    return 0;
}
