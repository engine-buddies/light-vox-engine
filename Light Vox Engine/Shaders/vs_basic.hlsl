#define _SHADER
#define _VSHADER
#include "../PipelineDefinitions.h"

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);


PSInput VSMain(
    VSInput vInput,
    uint instanceID : SV_InstanceID)
{
    //Declare PSInput
    PSInput result;

    //define position and world position
    float4 pos = float4(vInput.position, 1.0f);

    pos = mul(pos, gInstanceData[instanceID].model);

    result.worldpos = pos;

    //move to screen space
    pos = mul(pos, cView);
    pos = mul(pos, cProjection);
    result.position = pos;

    //supply uv and normal
    result.uv = vInput.uv;
    result.normal = mul(vInput.normal, (float3x3)gInstanceData[instanceID].model);

    return result;
}
