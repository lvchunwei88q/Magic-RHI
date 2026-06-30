#include "RHIRootSignatureVulKan.h"
#include "RHIVulKan.h"

namespace RHI
{
    namespace
    {
    }

    std::shared_ptr<RHIRootSignature> DeviceVulKan::CreateRootSignature(const RootSignatureDesc& desc)
    {
        return nullptr;
    }

    void DeviceVulKan::DeleteRootSignature(std::shared_ptr<RHIRootSignature>& rootSignature)
    {
        if (rootSignature)
        {
            rootSignature->Shutdown();
            rootSignature.reset();
        }
    }
}
