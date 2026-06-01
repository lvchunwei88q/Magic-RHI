#include "Function.hlsl"

RWTexture2D<float4> g_texture : register(u0);

// 线程组大小：16x16（常见选择）
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // 获取当前线程处理的像素坐标
    uint2 pixelPos = dispatchThreadID.xy;
    g_texture[pixelPos] = float4(1.0f, 0.0f, 0.0f, 1.0f);
}