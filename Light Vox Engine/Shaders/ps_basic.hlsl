//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

struct PSOutput
{
	float4 albedo   : SV_TARGET0;
	float4 normal   : SV_TARGET1;
    float4 position : SV_TARGET2;
};

PSOutput PSMain( PSInput input )
{
    PSOutput output;

    //re-normalize after rasterization
    input.normal = normalize(input.normal);

    //TO DO: Texturing:
    output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.normal = float4(input.normal, 1.0f);
    output.position = input.worldpos;

    return output;
}
