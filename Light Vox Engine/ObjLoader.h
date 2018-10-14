#pragma once
#include <DirectXMath.h>
#include <vector>

//TODO - move this out (I tried putting it in ShaderDefinitions.h but it broke stuff)
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
};

class ObjLoader
{
public:
	static void LoadObj(std::vector<Vertex>* vertices, 
        std::vector<uint16_t>* indices, 
        std::string filePath);
};