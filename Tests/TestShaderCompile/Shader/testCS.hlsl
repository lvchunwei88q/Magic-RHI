#include "Common.hlsli"

cbuffer ComputeConstants : register(b0)
{
    uint ElementCount;
};

RWStructuredBuffer<uint> OutputData : register(u1);

[numthreads(8, 1, 1)]
void MAIN_FUNCTION(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < ElementCount)
    {
        OutputData[dispatchThreadID.x] = dispatchThreadID.x;
    }
}
