#include <iostream>
#include <windows.h>
#include <cstring>
#include "RHI/Public/RHI.h"

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
    RHI::RHIType type = RHI::RHIType::DirectX12;
    
    if (argc > 1 && strcmp(argv[1], "dx12") == 0) {
        type = RHI::RHIType::DirectX12;
        std::cout << "Using DirectX12" << std::endl;
    } else {
        std::cout << "Using DirectX11" << std::endl;
    }

    const char CLASS_NAME[] = "RHIWindowClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        type == RHI::RHIType::DirectX11 ? "RHI DirectX11 Window" : "RHI DirectX12 Window",
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
        swapChainDesc.BufferCount = 2;
        swapChainDesc.VSync = false;
        
        auto swapChain = RHI::SwapChain::Create(type);
        if (swapChain && swapChain->Initialize(device.get(), swapChainDesc))
        {
            std::cout << "SwapChain created successfully!" << std::endl;
            
            MSG msg = {};
            while (GetMessage(&msg, nullptr, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
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
