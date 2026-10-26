#include "Common.hlsli"
struct VSInput
{
    float4 position : POSITION;
    float4 color    : COLOR;
};

struct VSOutput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
};

VSOutput MAIN_FUNCTION(VSInput input)
{
    VSOutput output;
    output.pos   = input.position;
    output.color = input.color;
    return output;
}