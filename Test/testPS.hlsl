#include "Function.hlsl"

// PS 输入来自 VS 输出
struct PSInput
{
    float4 pos : SV_POSITION;      // 屏幕空间位置
    float4 Color : COLOR;          // 颜色
};

float4 main(PSInput input) : SV_Target
{
    float4 result = input.Color;   
    return result;
}