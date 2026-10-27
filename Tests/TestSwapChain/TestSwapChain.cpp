#include <Subsystem/Subsystem.h>
#include <iostream>
#include <windows.h>
#include <CoreMinimal.h>
#include <RHI.hpp>
#include <IO.h>
#include <chrono>
#include "../Common/Test.h"

uint32_t x, y;
bool ExeSetSize = false;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY: return 0;
    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        if (width > 0 && height > 0) { x = width; y = height; ExeSetSize = true; }
        return 0;
    }
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void RunTestDeviceAndSwapChain(RHI::RHIType rhiType,const std::wstring& windowTitle)
{
    Core::SubsystemControl::Init();
    {
        std::wstring windowName = L"RHIWindowClass_" + windowTitle;
        WNDCLASS wc = {0};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = windowName.c_str();
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(0, windowName.c_str(), windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        ASSERT_TRUE(hwnd != nullptr);
        ShowWindow(hwnd, SW_SHOW);

        RHIAPILoader* loader = RHIModule::GetRHILoader();
        ASSERT_TRUE(loader->Load(rhiType));

        auto device = loader->CreateDevice();
        ASSERT_TRUE(device && device->Initialize() && device->IsValid());

        RHI::SwapChainDesc swapChainDesc = {};
        swapChainDesc.WindowHandleRef = hwnd;
        swapChainDesc.Width = 800;
        swapChainDesc.Height = 600;
        swapChainDesc.VSync = false;

        auto swapChain = loader->CreateSwapChain();
        ASSERT_TRUE(swapChain && swapChain->Initialize(device.get(), swapChainDesc) && swapChain->IsValid());

        std::vector<std::shared_ptr<RHI::RHICommandPool>> cmdPools (RHI_MULTI_BUFFERING);
        std::vector<std::shared_ptr<RHI::RHICommandList>> cmdLists (RHI_MULTI_BUFFERING);

        for (size_t i = 0; i < RHI_MULTI_BUFFERING; ++i)
        {
            cmdPools[i] = device->CreateCommandPool (RHI::RHICmdType::Graphics);
            cmdLists[i] = device->CreateCommandList (cmdPools[i]);
            ASSERT_TRUE (cmdLists[i] && cmdLists[i].get () != nullptr);
        }

        MSG msg = {};
        auto startTime = std::chrono::steady_clock::now();

        while (true){
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }


            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            if (elapsed >= 5) 
            {
                std::cout << "5 seconds elapsed. Closing automatically..." << std::endl;
                break; // break the loop after 5 seconds
            }

            device->GetFrameCommandQueue()->BeginFrame();
            uint64_t ImageIndex = swapChain->AvailableSwapChainImageIndex ();
            device->GetFrameCommandQueue()->ResetCommandPoolMemory(cmdPools[ImageIndex].get ());
            RHI::GraphicsQueueSwapChainInfo GQSCInfo = swapChain->GetGraphicsQueueSwapChainInfo();
            device->GetFrameCommandQueue()->SetGraphicsQueueSwapChainInfo(GQSCInfo);

            cmdLists[ImageIndex]->BeginRecording ();
            uint32_t SwapChainImageIndex = swapChain->AvailableSwapChainImageIndex();
            RHI::RHITexture* backBuffer = swapChain->GetBackBuffer(SwapChainImageIndex);

            RHI::BarrierDesc barrier = {};
            barrier.Type = RHI::ResourceBarrierType::Transition;
            barrier.ResourceType = RHI::BarrierResourceType::Texture;
            barrier.Transition.pResource = backBuffer;
            barrier.Transition.StateBefore = RHI::RHIResourceState::Present;
            barrier.Transition.StateAfter = RHI::RHIResourceState::RenderTarget;
            cmdLists[ImageIndex]->ResourceBarrier(1, &barrier);

            RHI::RHIRenderTargetView* pRTV = swapChain->GetRenderTargetView(SwapChainImageIndex);
            cmdLists[ImageIndex]->OMSetRenderTargets(1, &pRTV, false, nullptr);
            
            float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f }; // 灰色
            cmdLists[ImageIndex]->ClearRenderTargetView(pRTV, clearColor);

            barrier.Transition.StateBefore = RHI::RHIResourceState::RenderTarget;
            barrier.Transition.StateAfter = RHI::RHIResourceState::Present;
            cmdLists[ImageIndex]->ResourceBarrier(1, &barrier);
            cmdLists[ImageIndex]->EndRecording();

            device->GetFrameCommandQueue()->ExecuteCommandLists({ cmdLists[ImageIndex] });
            device->GetFrameCommandQueue()->Present();

            device->GetFrameCommandQueue()->EndFrame();

            if (ExeSetSize) 
            { 
                ExeSetSize = false;
                device->GetFrameCommandQueue()->WaitForGPU();
                swapChain->Resize(x, y);
            }
        }

        // Cleanup
        if (hwnd) {
            device->GetFrameCommandQueue()->WaitForGPU();
            DestroyWindow(hwnd);
            hwnd = nullptr;
        }
        UnregisterClass(windowName.c_str(), GetModuleHandle(nullptr));
    }
    Core::SubsystemControl::Uninstall();

    // Wait a moment before exiting to make sure Windows has finished its asynchronous cleanup.
    Sleep(10); 
}

void TestDeviceAndSwapChain()
{
    RHI::RHIType type = RHI::GetBestAvailableRHI();
    int testCount = 0;

    if (EnumHasAnyFlags (type, RHI::RHIType::VulKan)) {
        std::cout << "Running SwapChain Test with Vulkan..." << std::endl;
        RunTestDeviceAndSwapChain (RHI::RHIType::VulKan, L"Vulkan SwapChain Test");
        testCount++;
    }
    if(EnumHasAnyFlags(type, RHI::RHIType::D3D12)) {
        std::cout << "Running SwapChain Test with D3D12..." << std::endl;
        RunTestDeviceAndSwapChain(RHI::RHIType::D3D12, L"D3D12 SwapChain Test");
        testCount++;
    }
    
    if (testCount == 0) {
        std::cerr << "No suitable RHI found!" << std::endl;
    }
}

TEST_MAIN(TestDeviceAndSwapChain)