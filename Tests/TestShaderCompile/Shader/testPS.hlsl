#include "Common.hlsli"

cbuffer PixelConstants : register(b0)
{
    float4 Tint;
};

Texture2D<float4> ColorTexture : register(t1);
SamplerState ColorSampler : register(s2);

struct PSInput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
    float2 uv    : TEXCOORD0;
};

float4 MAIN_FUNCTION(PSInput input) : SV_Target
{
    return ColorTexture.Sample(ColorSampler, input.uv) * input.color * Tint;
}