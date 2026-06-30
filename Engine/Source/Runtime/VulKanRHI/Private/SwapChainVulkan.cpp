#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    SwapChainVulKan::SwapChainVulKan()
    {
    }

    SwapChainVulKan::~SwapChainVulKan()
    {
    }
    
    bool SwapChainVulKan::Initialize(Device* device, const SwapChainDesc& desc)
    {
        return true;
    }

    void SwapChainVulKan::Shutdown()
    {
    }

    bool SwapChainVulKan::IsValid() const
    {
        return true;
    }

    void SwapChainVulKan::Present(uint32_t syncInterval, uint32_t presentFlags)
    {
    }

    void SwapChainVulKan::Resize(uint32_t width, uint32_t height)
    {
    }

    RHIRenderTargetView* SwapChainVulKan::GetRenderTargetView(uint32_t index) const
    {
        return nullptr;
    }

    RHITexture* SwapChainVulKan::GetBackBuffer(uint32_t index) const
    {
        return nullptr;
    }
}
