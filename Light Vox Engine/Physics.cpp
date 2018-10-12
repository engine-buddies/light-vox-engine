#include "Physics.h"

Physics::Physics()
{
    entityManager = EntityManager::GetInstance();
}

void Physics::Init()
{
    gravity = { .0f, .0f, .0f };
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

}

void Physics::AccumlateForces()
{
}



