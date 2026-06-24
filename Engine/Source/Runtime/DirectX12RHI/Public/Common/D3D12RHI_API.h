// RHI/Public/Common/RHI_API.h
#pragma once

#ifdef D3D12RHI_EXPORTS
#define D3D12RHI_API __declspec(dllexport)
#else
#define D3D12RHI_API __declspec(dllimport)
#endif