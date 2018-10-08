#pragma once
#include <DirectXMath.h>
#include <vector>

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
};

class ObjLoader
{
public:
	static void LoadObj(std::vector<Vertex>* vertices, std::vector<uint16_t>* indices, std::string filePath);
};