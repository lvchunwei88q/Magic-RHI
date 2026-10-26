#include "Common.hlsli"

struct PSInput
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR;
};

float4 MAIN_FUNCTION(PSInput input) : SV_Target
{
    return float4(input.color.rgb, 1.0f);
}