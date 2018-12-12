#ifndef SHADER_DEF_H
#define SHADER_DEF_H

/* This file is meant for graphics/pipeline defintions that is shared between
*  GPU/shader and cpu-type pipeliney stuff
*/

#ifndef _SHADER

namespace Graphics
{
#define LV_NUM_GBUFFER_RTV 3
#define LV_NUM_NULL_SRV LV_NUM_GBUFFER_RTV
#define LV_NUM_RTV_PER_FRAME (LV_NUM_GBUFFER_RTV + 1)
#define LV_NUM_CBVSRV_PER_FRAME (LV_NUM_GBUFFER_RTV + 2)

    //indices to the per-frame command list
#define LV_COMMAND_LIST_COUNT 2
#define LV_COMMAND_LIST_INIT 0
#define LV_COMMAND_LIST_LIGHTING_PASS 1

//Root signature
#define LV_ROOT_SIGNATURE_INSTANCED_DATA 0
#define LV_ROOT_SIGNATURE_GBUFFER_SRV 1
#define LV_ROOT_SIGNATURE_CBV 2
#define LV_ROOT_SIGNATURE_SAMPLER 3

#define LV_ROOT_SIGNATURE_COUNT 4

    /// <summary>
    /// This is a helper class for initialization (it keeps our state machine 
    /// configuration all in one place
    /// </summary>
    class ShaderDefinitions
    {
    public:
        static inline void SetGeometryPassInputLayout( D3D12_INPUT_ELEMENT_DESC* inputLayout );
        static inline void SetLightingPassInputLayout( D3D12_INPUT_ELEMENT_DESC* inputLayout );

        static const DXGI_FORMAT GeometryBufferFormat( size_t index ) {
            static const DXGI_FORMAT GEOMETRY_BUFFER_FORMAT[] = {
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_SNORM,
                DXGI_FORMAT_R16G16B16A16_FLOAT
            };
            return GEOMETRY_BUFFER_FORMAT[ index ];
        }
    };
}
#endif

// ----------------- INSTANCE BUFFER -----------------
#ifdef _VSHADER_GEOMETRY_PASS
struct InstanceData
{
    float4x4 model;
};
#endif
#ifndef _SHADER
/// <summary>
/// Buffer of data needed for a per-instance basis
/// </summary>
namespace Graphics
{
    struct InstanceBuffer
    {
        glm::mat4x4 model;
    };
}
#endif

// ----------------- LIGHTING CONSTANTS -----------------
#ifdef _PSHADER_LIGHTING_PASS

#define LV_POINT_LIGHT_COUNT 64

struct DirectionalLight
{
    float3 diffuseColor;
    float3 direction;
};

struct PointLight
{
    float3 diffuseColor;
    float3 position;
};

cbuffer SceneConstantBuffer : register( b0 )
{
    float4 cDiffuseColor[ LV_POINT_LIGHT_COUNT ];
    float4 cPosition[ LV_POINT_LIGHT_COUNT ];
};
#endif

// ----------------- CONSTANT BUFFER (PER-SCENE) -----------------
#if defined _VSHADER_GEOMETRY_PASS || defined _PSHADER_LIGHTING_PASS || defined _VSHADER_DEBUG
cbuffer SceneConstantBuffer : register( b0 )
{
    float4x4 cView;
    float4x4 cProjection;
    float3 cCameraPosition;
};
#endif

#ifndef _SHADER
/// <summary>
/// Buffer of data needed for a per-scene basis
/// </summary>
namespace Graphics
{

    struct SceneConstantBuffer
    {
        glm::mat4x4_packed view;
        glm::mat4x4_packed projection;
        glm::vec3_packed cameraPosition;
    };

}
#endif

// ----------------- INPUT ASSEMBLER (GEOMETRY) -----------------
#ifdef _VSHADER_GEOMETRY_PASS
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};
#endif
#ifndef _SHADER
//number of input variables for the basic VS
#define LV_NUM_VS_INPUT_COUNT 3
namespace Graphics
{
    struct Vertex
    {
        glm::vec3_packed position;
        glm::vec3_packed normal;
        glm::vec2_packed uv;
    };
}

/// <summary>
/// Defer input layout logic to PipelineDefinitions (so that's it's easier to 
/// sync with how the shader is defined).
/// </summary>
inline void Graphics::ShaderDefinitions::SetGeometryPassInputLayout( D3D12_INPUT_ELEMENT_DESC* inputLayout )
{
    //semantic name, index, format, input slot, data offset, input classification, instance rate
    inputLayout[ 0 ] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputLayout[ 1 ] = { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputLayout[ 2 ] = { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}
#endif

// ----------------- VS TO PS (GEOMETRY) -----------------
#if defined _VSHADER_GEOMETRY_PASS || defined _PSHADER_GEOMETRY_PASS
struct VStoPS
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};
#endif

// ----------------- PS Output (GEOMETRY) -----------------
#ifdef _PSHADER_GEOMETRY_PASS
struct PSOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
};
#endif
#ifdef _PSHADER_LIGHTING_PASS
#define _G_BUFFER_ABLEDO_REG t0
#define _G_BUFFER_NORMAL_REG t1
#define _G_BUFFER_POSITION_REG t2
#endif

// ----------------- INPUT ASSEMBLER (LIGHTING) -----------------
// TODO - actually use this in code
#ifdef _VSHADER_LIGHTING_PASS
struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};
#endif
#ifndef _SHADER
/// <summary>
/// Defer input layout logic to PipelineDefinitions (so that's it's easier to 
/// sync with how the shader is defined).
/// </summary>
inline void Graphics::ShaderDefinitions::SetLightingPassInputLayout( D3D12_INPUT_ELEMENT_DESC* inputLayout )
{
    //semantic name, index, format, input slot, data offset, input classification, instance rate
    inputLayout[ 0 ] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputLayout[ 1 ] = { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}
#endif

// ----------------- VS TO PS (LIGHTING) -----------------
#if defined _VSHADER_LIGHTING_PASS || defined _PSHADER_LIGHTING_PASS
struct VStoPS
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};
#endif


// ----------------- VS TO PS (DEBUG) -----------------
#if defined _VSHADER_DEBUG || defined _PSHADER_DEBUG
struct VStoPS
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};
#endif

#endif// SHADER_DEF_H
