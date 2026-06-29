#include "Function.hlsl"

// PS 输入来自 VS 输出
struct PSInput
{
    float4 pos : SV_POSITION;      // 屏幕空间位置
    float4 Color : TEXCOORD1;          // 颜色
};

uint4 ConvertToPixel(float4 ndcPos, uint2 viewportSize)
{
    uint4 pixelPos;
    pixelPos.xy = (uint)((ndcPos.xy * 0.5f + 0.5f) * viewportSize);  // 转换为整数
    pixelPos.zw = (uint)ndcPos.zw;
    return pixelPos;
}

struct Vertex
{
    float4 Position;  // XYZW
    float4 Color;     // RGBA
};

cbuffer VertexBuffer : register(b2)
{
    Vertex Vertices[1024];  // 大小固定
};

#if SHADER_MODEL > 50
cbuffer DrawConstants : register(b3, space0)
#else
cbuffer DrawConstants : register(b3)
#endif
{
    float value;
};

float4 main(PSInput input) : SV_Target
{
    //float4 result =  value * 100.0f * input.Color * SHADER_MODEL / 50.0f;
    float result1 = Vertices[input.Color.z * 64].Color.x;
    float result2 = Vertices[input.Color.y * 64].Color.y;
    float result3 = Vertices[input.Color.x * 64].Color.z;
#if SHADER_MODEL > 50
    return float4(result1, result2, result3, 1.0f) * SHADER_MODEL / 50.0f * value;
#else
    return input.Color * SHADER_MODEL / 50.0f * value;
#endif
}