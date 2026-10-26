// PS 输入来自 VS 输出
struct PSInput
{
    float4 pos : SV_POSITION;      // 屏幕空间位置
    float4 Color : TEXCOORD1;          // 颜色
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.Color.rgb,1.0f); // 输出颜色，保持 alpha 为 1.0
}