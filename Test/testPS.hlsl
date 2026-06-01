#include "Function.hlsl"

// PS 输入来自 VS 输出
struct PSInput
{
    float4 pos : SV_POSITION;      // 屏幕空间位置
};

float4 main(PSInput input) : SV_Target
{
    float4 result = float4(1.0f, 0.0f, 0.0f, 1.0f);   
    return result;
}