//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _VSHADER_SKYBOX

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

VStoPS main(
    VSInput input)
{
    // Set up output
    VStoPS output;

    matrix viewNoTranslation = cView;
    viewNoTranslation._41 = 0;
    viewNoTranslation._42 = 0;
    viewNoTranslation._43 = 0;

    // Calculate output position
    matrix viewProj = mul(viewNoTranslation, cProjection);
    output.position = mul(float4(input.position, 1.0f), viewProj);

    // Push the depth of the vertex to the far clip plane
    output.position.z = output.position.w;

    // Determine my sample direction for when we 
    // sample from the cube map in the pixel shader
    output.sampleDir = input.position;
    // Assuming the position is also a direction in space towards that vertex


    return output;
}