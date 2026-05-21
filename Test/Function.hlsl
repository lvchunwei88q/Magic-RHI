#include "common.hlsl"
#include "math.hlsl"

float4 XColor(float4 input, float intensity)
{
    return ApplyColor(input, intensity) * MultiplyByTwo(input);
}