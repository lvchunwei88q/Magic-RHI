struct VSInput
{
    float4 position : POSITION; // Corresponds to InputLayout[0]
    float4 color : COLOR; // Corresponds to InputLayout[1]
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 Color : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput result;
    result.pos      = input.position;
    result.Color    = input.color;
    return result;
}