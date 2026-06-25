// RHI/Public/Common/RHI_API.h
#pragma once

#ifdef D3D11RHI_EXPORTS
#define D3D11RHI_API __declspec(dllexport)
#else
#define D3D11RHI_API __declspec(dllimport)
#endif