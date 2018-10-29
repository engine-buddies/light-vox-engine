//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER_GEOMETRY_PASS

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

PSOutput PSMain( VStoPS input )
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
