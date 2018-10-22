#pragma once

#include "stdafx.h"
#include <DirectXMath.h>
#include <vector>

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;
};
/*
struct ObjectData
{
    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* indexBuffer = nullptr;
    UINT indexCount = 0;

    ~ObjectData()
    {
        delete vertexBuffer;
        delete indexBuffer;
    }
};
*/
class ObjLoader
{
public:
	static void LoadObj(std::vector<Vertex>* vertices, std::vector<uint16_t>* indices, std::string filePath);
    
    /// <summary>
    /// Generates a Full Screen Quad 
    /// used for deferred directional light calculations 
    /// </summary>
    void GenerateFullScreenQuad(MeshData& meshData);
};