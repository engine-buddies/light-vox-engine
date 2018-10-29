//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _VSHADER_LIGHTING_PASS

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

VStoPS main(
	float3 position : POSITION,
	float2 uv : TEXCOORD0,
	float3 normal : NORMAL)
{
    VStoPS output;
	output.position = float4(position, 1.f);
	output.uv = uv;
	return output;
}
