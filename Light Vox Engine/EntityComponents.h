#pragma once
#include <DirectXMath.h>
/// <summary>
/// Structures of the different components that an entity can have
/// </summary>
/// <author>Arturo</author>
/// <author>Ben Hoffman</author>
namespace EntityComponents
{
    struct Transform
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 rot;
        DirectX::XMMATRIX transformMatrix;
    };

    struct BoxCollider
    {
        DirectX::XMFLOAT3 size;
    };
};
