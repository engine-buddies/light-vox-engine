
#ifndef SHADER_DEF_H
#define SHADER_DEF_H

/* This file is meant for graphics/pipeline defintions that is shared between
*  GPU/shader and cpu-type pipeliney stuff
*/

#ifndef _SHADER
//forward declare things we need to return
struct D3D12_INPUT_ELEMENT_DESC;

/// <summary>
/// This is a helper class for initialization (it keeps our state machine 
/// configuration all in one place
/// </summary>
class ShaderDefinitions
{
public:
    static inline void SetInputLayout( D3D12_INPUT_ELEMENT_DESC* inputLayout );
};
#endif

// -- INSTANCING ---
#ifdef _SHADER
struct InstanceData
{
    float4x4 model;
};
#endif
#ifndef _SHADER
/// <summary>
/// Buffer of data needed for a per-instance basis
/// </summary>
struct InstanceBuffer
{
    DirectX::XMFLOAT4X4 model;
};
#endif

// -- PER-SCENE CONSTANT BUFFER ---
#ifdef _VSHADER
cbuffer SceneConstantBuffer : register( b0 )
{
    float4x4 cView;
    float4x4 cProjection;
};
#endif
#ifndef _SHADER
/// <summary>
/// Buffer of data needed for a per-scene basis
/// </summary>
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
};
#endif

// -- INPUT ASSEMBLER ---
#ifdef _VSHADER
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

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
};

/// <summary>
/// Defer input layout logic to PipelineDefinitions (so that's it's easier to 
/// sync with how the shader is defined).
/// </summary>
inline void ShaderDefinitions::SetInputLayout( D3D12_INPUT_ELEMENT_DESC* inputLayout )
{
    //semantic name, index, format, input slot, data offset, input classification, instance rate
    inputLayout[ 0 ] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; 
    inputLayout[ 1 ] = { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputLayout[ 2 ] = { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}
#endif

// -- VS TO PS ---
#ifdef _SHADER
struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};
#endif

#endif// SHADER_DEF_H