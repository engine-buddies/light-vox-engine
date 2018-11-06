#include "Solver.h"

using namespace Physics;

Solver::Solver()
{
    componentManager = ECS::ComponentManager::GetInstance();
    rigidBody = new Rigidbody();
    gravity = { .0f, .0f, .0f };
}

Solver::~Solver()
{
    delete rigidBody;
}

void Solver::Update(float dt)
{
    SetColliderData();
    Collide();
    AccumlateForces();
    AccumlateTorque();
    Integrate(dt);
    ModelToWorld();
    //SatisfyConstraints();
}

void Solver::Collide()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        for (size_t j = 0; j < LV_MAX_INSTANCE_COUNT; ++j)
        {
            if (i == j)
                continue;

            rigidBody->IntersectBoxBox(i, j);
            //if (rigidBody->CollideBoxBox(i, j)) printf("hit");
            /* glm::vec3& posA = componentManager->transform[ i ].pos;
             glm::vec3& posB = componentManager->transform[ j ].pos;
             glm::vec3& sizeA = componentManager->boxCollider[ i ].size;
             glm::vec3& sizeB = componentManager->boxCollider[ j ].size;
             if ( BoxIntersect( posA, posB, sizeA, sizeB ) )
             {
                 DEBUG_PRINT("Entity: %i hit Entity: %i \n", i, j);
             }*/

        }
    }
}

void Solver::Integrate(float dt)
{
    //semi implicit euler 
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        if (!componentManager->bodyProperties[i].isAwake)
            return;

        //movement
        glm::vec3& acceleration = componentManager->bodyProperties[i].acceleration;
        glm::vec3& velocity = componentManager->bodyProperties[i].velocity;
        glm::vec3& position = componentManager->transform[i].pos;
        glm::vec3& force = componentManager->bodyProperties[i].force;
        //rotation
        glm::vec3& angularAccel = componentManager->bodyProperties[i].angularAcceleration;
        glm::vec3& rot = componentManager->transform[i].rot;
        glm::quat& orientation = componentManager->transform[i].orientation;
        glm::vec3& torque = componentManager->bodyProperties[i].torque;

        //euler integration for movement and rotation 
        velocity += acceleration * dt;
        rot += angularAccel * dt;

        //impose drag
        //velocity *= glm::pow(.80, dt);
        //rot *= glm::pow(.80, dt);

        position += velocity * dt;

        // Angular vel. formula for quaternions 
        // 0' = 0 + (dt / 2) * w * 0
        // 0  = old rotation
        // 0' = new rotation
        // w  = (quaternion) [0 wx wy wz]
        glm::quat q = glm::quat(
            0,
            rot.x * dt,
            rot.y * dt,
            rot.z * dt);

        q = q * orientation;

        orientation += (q * .5f);

        //clear torque and forces
        force = { .0f, .0f, .0f };
        torque = { .0f, .0f, .0f };
    }
}

void Solver::AccumlateForces()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        glm::vec3& acceleration = componentManager->bodyProperties[i].acceleration;
        glm::vec3& force = componentManager->bodyProperties[i].force;
        float& invMass = componentManager->bodyProperties[i].invMass;
        acceleration += force * invMass;
    }
}

void Physics::Solver::AccumlateTorque()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        glm::vec3& angularAccel = componentManager->bodyProperties[i].angularAcceleration;
        glm::vec3& torque = componentManager->bodyProperties[i].torque;
        glm::mat3& invInertiaTensor = componentManager->bodyProperties[i].inertiaTensor;
        angularAccel += invInertiaTensor * torque;
    }
}



void Solver::ModelToWorld()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        glm::mat4& transformMatrix = componentManager->transform[i].transformMatrix;
        glm::vec3& pos = componentManager->transform[i].pos;
        glm::quat& orientation = componentManager->transform[i].orientation;
        orientation = glm::normalize(orientation);
        glm::vec3& scale = componentManager->transform[i].scale;

        transformMatrix =
            glm::translate(pos) *
            glm::toMat4(orientation) *
            glm::scale(scale);
    }
}

void Physics::Solver::SetColliderData()
{
    for (UINT i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        rigidBody->CalcHalfSize(i);

    }
}
