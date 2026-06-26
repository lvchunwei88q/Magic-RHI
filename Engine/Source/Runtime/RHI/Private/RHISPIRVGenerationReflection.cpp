// RHI/Private/RHISPIRVGenerationReflection.cpp
#include "CoreMinimal.h"
#include "RHIShaderCompiler.h"

// SPI SPIRV-Cross headers
#include <spirv_cross.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_common.hpp>

namespace RHI {

SPIRVReflection SPIRVGenerationReflection::ExtractReflection(const std::vector<uint32_t>& spirv) {
    SPIRVReflection reflection;

    // Check input SPIR-V data
    if (spirv.empty() || spirv.size() < 5) {
        return reflection;
    }

    // Check SPIR-V magic number (0x07230203) - A magic number is used to indicate that this is proprietary SPIRV data.
    if (spirv[0] != 0x07230203) {
        Core::ErrorCapture::Capture("SPI SPIR-V magic number not found.");
        return reflection;
    }

    try {
        // Create HLSL compiler
        spirv_cross::CompilerHLSL compiler(spirv);

        // Get entry points using get_entry_points_and_stages()
        auto entryPoints = compiler.get_entry_points_and_stages();
        if (!entryPoints.empty()) {
            reflection.entryPoint = entryPoints[0].name;
        }

        // Get all shader resources using get_shader_resources()
        auto resources = compiler.get_shader_resources();

        // ============================================================
        // Extract Constant Buffers (cbuffer)
        // ============================================================
        for (auto& resource : resources.uniform_buffers) {
            SPIRVResource res;
            res.name = resource.name;
            res.isConstantBuffer = true;
            res.isTexture = false;
            res.isSampler = false;

            // Get binding information: set = space, binding = register
            res.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            res.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

            // Get size of constant buffer
            auto& type = compiler.get_type(resource.base_type_id);
            res.size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

            // Extract member variables
            auto memberCount = type.member_types.size();
            for (uint32_t i = 0; i < memberCount; ++i) {
                auto memberName = compiler.get_member_name(resource.base_type_id, i);
                auto memberOffset = compiler.type_struct_member_offset(type, i);
                res.members.push_back({memberName, memberOffset});
            }

            reflection.resources.push_back(res);
            reflection.resourcesBySet[res.set].push_back(res);
            reflection.totalConstantBufferSize += res.size;
        }

        // ============================================================
        // Extract textures: (Texture2D, TextureCube, etc.)
        // ============================================================
        for (auto& resource : resources.separate_images) {
            SPIRVResource res;
            res.name = resource.name;
            res.isConstantBuffer = false;
            res.isTexture = true;
            res.isSampler = false;

            res.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            res.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            res.size = 0;

            reflection.resources.push_back(res);
            reflection.resourcesBySet[res.set].push_back(res);
        }

        // ============================================================
        // Extract samplers: (SamplerState)
        // ============================================================
        for (auto& resource : resources.separate_samplers) {
            SPIRVResource res;
            res.name = resource.name;
            res.isConstantBuffer = false;
            res.isTexture = false;
            res.isSampler = true;

            res.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            res.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            res.size = 0;

            reflection.resources.push_back(res);
            reflection.resourcesBySet[res.set].push_back(res);
        }

        // ============================================================
        // Extract Storage Buffers (RWStructuredBuffer, Buffer, etc.)
        // ============================================================
        for (auto& resource : resources.storage_buffers) {
            SPIRVResource res;
            res.name = resource.name;
            res.isConstantBuffer = false;
            res.isTexture = false;
            res.isSampler = false;

            res.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            res.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

            auto& type = compiler.get_type(resource.base_type_id);
            res.size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

            reflection.resources.push_back(res);
            reflection.resourcesBySet[res.set].push_back(res);
        }

        // ============================================================
        // Extract Unordered Access Views (RWTexture2D, etc.)
        // ============================================================
        for (auto& resource : resources.storage_images) {
            SPIRVResource res;
            res.name = resource.name;
            res.isConstantBuffer = false;
            res.isTexture = true;
            res.isSampler = false;

            res.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            res.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            res.size = 0;

            reflection.resources.push_back(res);
            reflection.resourcesBySet[res.set].push_back(res);
        }

        // ============================================================
        // Extract Push Constants: (PushConstantBuffer)
        // ============================================================
        for (auto& resource : resources.push_constant_buffers) {
            SPIRVResource res;
            res.name = resource.name;
            res.isConstantBuffer = true;
            res.isTexture = false;
            res.isSampler = false;

            res.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            res.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

            auto& type = compiler.get_type(resource.base_type_id);
            res.size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

            reflection.resources.push_back(res);
            reflection.resourcesBySet[res.set].push_back(res);
            reflection.totalConstantBufferSize += res.size;
        }

    } catch (const std::exception& e) {
        // SPIRV-Cross Parsing failed, return empty reflection
        Core::ErrorCapture::Capture("SPIRV-Cross Parsing failed: " + std::string(e.what()));
        return SPIRVReflection{};
    }

    return reflection;
}

} // namespace RHI