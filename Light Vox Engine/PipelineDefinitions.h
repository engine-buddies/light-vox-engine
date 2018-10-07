#pragma once

#ifndef _SHADER

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
/// Whatever goes into the constant buffer for our main 'scene' render
/// </summary>
struct InstanceBuffer
{
    DirectX::XMFLOAT4X4 model;
};
#endif



// -- PER-SCENE CONSTANT BUFFER ---
#ifdef _VSHADER
cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 cView;
    float4x4 cProjection;
};
#endif
#ifndef _SHADER
/// <summary>
/// Whatever goes into the constant buffer for our main 'scene' render
/// </summary>
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
};
#endif

