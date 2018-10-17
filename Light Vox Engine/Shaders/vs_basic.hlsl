//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _VSHADER

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

//basically a SRV of instanced data
StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);

PSInput VSMain(
    VSInput vInput,
    uint instanceID : SV_InstanceID)
{
    //Declare PSInput
    PSInput result;

    //define position and world position
    float4 pos = float4(vInput.position, 1.0f);
    float4x4 model = gInstanceData[instanceID].model;
    
    pos = mul(pos, model);
    result.worldpos = pos;

    //move to screen space
    pos = mul(pos, cView);
    pos = mul(pos, cProjection);
    result.position = pos;

    //supply uv and normal
    result.uv = vInput.uv;
    result.normal = mul(vInput.normal, (float3x3)model);
    return result;
}
