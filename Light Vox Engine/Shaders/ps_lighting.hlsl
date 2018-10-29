//these #defines are used to selectively compile from PipelineDefinitions.h
#define _SHADER
#define _PSHADER_LIGHTING_PASS

//contains all defitnitions for shader-to-shader and shader-to-CPU stuff
#include "../Graphics/ShaderDefinitions.h"

//Deferred SRVs
Texture2D gAlbedoTexture    : register( _G_BUFFER_ABLEDO_REG );
Texture2D gNormalTexture    : register( _G_BUFFER_NORMAL_REG );
Texture2D gPositionTexture  : register( _G_BUFFER_POSITION_REG );

//Sampler
SamplerState gSampler       : register( s0 );

/// <summary>
/// Helper to calculate diffuse for dir light
/// </summary>
float3 dirLightDiffuse( DirectionalLight dirLight, float3 normal ) {
    return dirLight.diffuseColor * saturate( dot( -dirLight.direction, normal ) );
};

/// <summary>
/// Helper to calculate diffuse for point light
/// </summary>
float3 pointLightDiffuse( PointLight pointLight, float3 normal, float3 worldPos ) {
    float3 posToPointLight = pointLight.position - worldPos;
    float3 dirToPointLight = normalize( posToPointLight );
    float pointLightAmount = saturate( dot( normal, dirToPointLight ) );
    return pointLight.diffuseColor * pointLightAmount / length( posToPointLight );
};


float4 main( VStoPS input ) : SV_TARGET
{
    float3 albedo = gAlbedoTexture.Sample( gSampler, input.uv ).xyz;
    float3 normal = gNormalTexture.Sample( gSampler, input.uv ).xyz;
    float3 worldPos = gPositionTexture.Sample( gSampler, input.uv ).xyz;

    //ambient
    float3 color = float3( 0.1, 0.1, 0.1 );

    PointLight pointLights[ 5 ];
    pointLights[ 0 ].diffuseColor = float3( 1, 0, 0);
    pointLights[ 0 ].position = float3( 2, 0, 0 );
    pointLights[ 1 ].diffuseColor = float3( 0, 1, 0);
    pointLights[ 1 ].position = float3( -5, 0, -1 );
    pointLights[ 2 ].diffuseColor = float3( 0, 0, 1);
    pointLights[ 2 ].position = float3( 1, 2, 0 );
    pointLights[ 3 ].diffuseColor = float3( 1, 1, 0);
    pointLights[ 3 ].position = float3( -2, -3, 0 );
    pointLights[ 4 ].diffuseColor = float3( 0, 1, 1 );
    pointLights[ 4 ].position = float3( -5, 2, -1 );

    //diffuse
    float3 diffuse = float3( 0, 0, 0 );
    [unroll]
    for ( int i = 0; i < 5; ++i )
        diffuse += pointLightDiffuse( pointLights[i], normal, worldPos );
    color += diffuse * albedo;

    return float4(color, 1.0);
}