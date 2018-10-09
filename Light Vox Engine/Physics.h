#pragma once
#include "Entity.h"

class Physics : public Component<Physics>
{
public:

    struct Transform : public Component<Transform> {
        DirectX::XMMATRIX transformMatrix = DirectX::XMMatrixIdentity();
        DirectX::XMFLOAT3 pos = { 0, 0, 0 };
        DirectX::XMFLOAT3 rot = { 0, 0, 0 };
        DirectX::XMFLOAT3 scale = { 0, 0, 0 };

    };

    struct BodyProperties : public Component<BodyProperties> {
        DirectX::XMFLOAT3 velocity = { 0, 0, 0 };
        float mass = 1.0f;

    };

    struct BoxCollider : public Component<BoxCollider>
    {
        DirectX::XMFLOAT3 size = {1.0, 1.0f, 1.0f};
    };

    struct SphereCollider: public Component<SphereCollider>
    {
        float radius = 1.0f;
    };
    

    void Init();
    void Update(double dt);
    bool Collide(std::vector<Entity> es);

private:
    Component<Transform> transform;
    Component<BodyProperties> bodyProperties;
};