// PS input comes from VS output
struct PSInput
{
    float4 pos : SV_POSITION;      // Screen space position
    float4 Color : TEXCOORD1;      // Color
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.Color.rgb,1.0f); // Output color, keep alpha as 1.0
}