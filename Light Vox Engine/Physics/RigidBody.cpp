#include "RigidBody.h"

Physics::Rigidbody::Rigidbody()
{
    componentManager = ECS::ComponentManager::GetInstance();
}

Physics::Rigidbody::~Rigidbody()
{
}

void Physics::Rigidbody::Pos(glm::vec3 pos, UINT index)
{
    componentManager->transform[index].pos = pos;
}

void Physics::Rigidbody::RotateAxisAngle(glm::vec3 rotationAxis, float angle, UINT index)
{
    componentManager->transform[index].rot = rotationAxis;
    componentManager->transform[index].angle = angle;
}

void Physics::Rigidbody::Velocity(glm::vec3 vel, UINT index)
{
    componentManager->bodyProperties[index].velocity = vel;
}

void Physics::Rigidbody::Acceleration(glm::vec3 accel, UINT index)
{
    componentManager->bodyProperties[index].acceleration= accel;
}

void Physics::Rigidbody::Force(glm::vec3 force, UINT index)
{
    componentManager->bodyProperties[index].force = force ;
}

void Physics::Rigidbody::Mass(float mass, UINT index)
{
    componentManager->bodyProperties[index].mass = mass;
}

void Physics::Rigidbody::Scale(glm::vec3 scale, UINT index)
{
    componentManager->transform[index].scale = scale;
}

void Physics::Rigidbody::BoxColliderSize(glm::vec3 size, UINT index)
{
    componentManager->boxCollider[index].size = size;
}
