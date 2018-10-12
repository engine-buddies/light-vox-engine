#pragma once
#include "EntityManager.h"
#include <DirectXMath.h>

class Physics
{
public:

    Physics();
    void Init();
    void Update(double dt);
    bool Collide();

private:
    EntityManager* entityManager;

    void Integrate(double dt);
    void AccumlateForces();
    DirectX::XMFLOAT3 gravity;
};