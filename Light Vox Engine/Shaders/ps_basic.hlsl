//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../ShaderDefinitions.h"

//[TEST]This is just here to make our demos look less ugly 
struct DEBUG_DirectionalLight
{
    float4 diffuseColor;
    float3 direction;
};

/// <summary>
/// [TEST] This is only here to make our demo look a little tastier
/// </summary>
float4 DEBUG_dirLightDiffuse(DEBUG_DirectionalLight dirLight, float3 normal) {
    return dirLight.diffuseColor * saturate(dot(-normalize(dirLight.direction), normal));
};

float4 PSMain(PSInput input) : SV_TARGET
{
    //re-normalize after rasterization
    input.normal = normalize(input.normal);

    //[TEST] Create a 'test' light to help make sure normal data is correct 
    DEBUG_DirectionalLight dirLight;
    dirLight.diffuseColor = float4(0.8, 0.8, 0.8, 1.0);
    dirLight.direction = float3(-0.5, -0.5, 0);

    //[TEST] to make sure normal data is correct 
    float4 lights = float4(0.1, 0.1, 0.1, 1.0)
        + DEBUG_dirLightDiffuse(dirLight, input.normal);

    return lights;
}
