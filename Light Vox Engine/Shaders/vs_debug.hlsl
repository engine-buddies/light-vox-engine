//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _VSHADER_DEBUG

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

struct InstanceData
{
    float4x4 model;
    float3 color;
};

StructuredBuffer<InstanceData> gInstanceData : register( t0, space1 );

VStoPS main(
	float3 position : POSITION,
	float2 uv : TEXCOORD0,
	float3 normal : NORMAL,
    uint instanceID : SV_InstanceID )
{
    VStoPS output;

    float4 pos = float4( position, 1.0f );
    float4x4 model = gInstanceData[ instanceID ].model;
    pos = mul( pos, model );
    pos = mul( pos, cView );
    pos = mul( pos, cProjection );

    output.position = pos;
    output.color = gInstanceData[ instanceID ].color;

	return output;
}
