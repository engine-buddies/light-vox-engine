#include "RigidBody.h"

Physics::Rigidbody::Rigidbody()
{
    componentManager = ECS::ComponentManager::GetInstance();
}

Physics::Rigidbody::~Rigidbody()
{
}

void Physics::Rigidbody::Pos(glm::vec3 pos, uint32_t index)
{
    componentManager->transform[index].pos = pos;
}

void Physics::Rigidbody::RotateAxisAngle(glm::vec3 rotationAxis, float angle, uint32_t index)
{
    componentManager->transform[ index ].rot = glm::rotate( componentManager->transform[ index ].rot, angle, rotationAxis );
}

void Physics::Rigidbody::Velocity(glm::vec3 vel, uint32_t index)
{
    componentManager->bodyProperties[index].velocity = vel;
}

void Physics::Rigidbody::Acceleration(glm::vec3 accel, uint32_t index)
{
    componentManager->bodyProperties[index].acceleration= accel;
}

void Physics::Rigidbody::Force(glm::vec3 force, uint32_t index)
{
    componentManager->bodyProperties[index].force = force ;
}

void Physics::Rigidbody::Mass(float mass, uint32_t index)
{
    componentManager->bodyProperties[index].mass = mass;
}

void Physics::Rigidbody::Scale(glm::vec3 scale, uint32_t index)
{
    assert( "Hey you can't scale a voxel" );
}

void Physics::Rigidbody::BoxColliderSize(glm::vec3 size, uint32_t index)
{
    componentManager->boxCollider[index].size = size;
}
