//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER_GEOMETRY_PASS

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}