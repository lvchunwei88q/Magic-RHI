#include "Function.hlsl"

// PS 输入来自 VS 输出
struct PSInput
{
    float4 pos : SV_POSITION;      // 屏幕空间位置
    float4 Color : COLOR;          // 颜色
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
    float4 result =  value * input.Color * SHADER_MODEL / 50.0f;
    return result;
}