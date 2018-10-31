//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER_DEBUG

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

float4 main( VStoPS input ) : SV_TARGET
{
    return float4( input.color, 1 );
}
