// RHI/Public/Common/RHI_API.h
#pragma once

#ifdef DIRECTX11RHI_EXPORTS
#define DIRECTX11RHI_API __declspec(dllexport)
#else
#define DIRECTX11RHI_API __declspec(dllimport)
#endif