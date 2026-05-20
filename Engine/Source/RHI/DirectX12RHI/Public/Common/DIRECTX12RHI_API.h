// RHI/Public/Common/RHI_API.h
#pragma once

#ifdef DIRECTX12RHI_EXPORTS
#define DIRECTX12RHI_API __declspec(dllexport)
#else
#define DIRECTX12RHI_API __declspec(dllimport)
#endif