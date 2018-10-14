#pragma once
#include <DirectXMath.h>
#include "Entity.h"
#include "ComponentManager.h"

class Physics
{
public:
    Physics();
    void Update(double dt);
    bool Collide();

private:

    void Integrate(double dt);
    void AccumlateForces();

    DirectX::XMFLOAT3 gravity;
    ComponentManager* componentManager;
};