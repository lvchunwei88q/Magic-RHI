#include <iostream>
#include <RHI.h>

int main()
{
    std::cout << "Initializing DirectX11 Device..." << std::endl;
    
    auto dx11Device = RHI::Device::Create(RHI::RHIType::DirectX11);
    if (dx11Device && dx11Device->Initialize())
    {
        std::wcout << L"DirectX11 Device initialized successfully: " << dx11Device->GetAdapterName() << std::endl;
    }
    else
    {
        std::cout << "Failed to initialize DirectX11 Device!" << std::endl;
    }

    std::cout << "\nInitializing DirectX12 Device..." << std::endl;
    
    auto dx12Device = RHI::Device::Create(RHI::RHIType::DirectX12);
    if (dx12Device && dx12Device->Initialize())
    {
        std::wcout << L"DirectX12 Device initialized successfully: " << dx12Device->GetAdapterName() << std::endl;
    }
    else
    {
        std::cout << "Failed to initialize DirectX12 Device!" << std::endl;
    }

    std::cin.get();
    return 0;
}
