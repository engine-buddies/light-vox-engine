//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER_SKYBOX

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

TextureCube SkyTexture  : register(t4);
SamplerState Sampler    : register(s0);

PSOutput main(VStoPS input)
{
    PSOutput output;

    output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;
}