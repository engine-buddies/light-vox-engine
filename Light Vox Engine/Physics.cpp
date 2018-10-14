#include "Physics.h"

Physics::Physics()
{
    gravity = { .0f, .0f, .0f };
    componentManager = ComponentManager::GetInstance();
}

void Physics::Update(double dt)
{
    
}

bool Physics::Collide()
{
    return false;
}

void Physics::Integrate(double dt)
{
    //semi implicit euler 
}

void Physics::AccumlateForces()
{
}



