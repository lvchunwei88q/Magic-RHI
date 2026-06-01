#include "Function.hlsl"

float4 main(float4 pos : POSITION) : SV_POSITION
{
    float4 result = pos;
    result = XColor(result, 1.0f);
    return result;
}