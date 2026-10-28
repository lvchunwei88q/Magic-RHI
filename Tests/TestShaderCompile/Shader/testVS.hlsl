#include "Common.hlsli"

cbuffer VertexConstants : register(b0)
{
    float4 PositionOffset;
};

struct VSInput
{
    float4 position : POSITION;
    float4 color    : COLOR;
};

struct VSOutput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
    float2 uv    : TEXCOORD0;
};

VSOutput MAIN_FUNCTION(VSInput input)
{
    VSOutput output;
    output.pos   = input.position + PositionOffset;
    output.color = input.color;
    output.uv    = input.position.xy * 0.5f + 0.5f;
    return output;
}