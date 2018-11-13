#include "Solver.h"

using namespace Physics;

Solver::Solver()
{
    gravity = { .0f, .0f, .0f };
    componentManager = ECS::ComponentManager::GetInstance();
    jobManager = Jobs::JobManager::GetInstance();
}

Solver::~Solver()
{
    jobManager = nullptr;
}

void Solver::Update( float dt )
{
    //Collide();
    static float deltaTime = 0.016f;

    jobManager->AddJob( this, &Physics::Solver::AccumlateForces, &deltaTime, 0 );

    //AccumlateForces( nullptr, 0 );

    //Integrate( &deltaTime, 0 );

    //ModelToWorld( nullptr, 0 );


    //SatisfyConstraints();
}

void Solver::Collide()
{
    //Basic box to box collision 
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        for ( size_t j = 0; j < LV_MAX_INSTANCE_COUNT; ++j )
        {
            if ( i == j )
                continue;

            glm::vec3& posA = componentManager->transform[ i ].pos;
            glm::vec3& posB = componentManager->transform[ j ].pos;

            glm::vec3& sizeA = componentManager->boxCollider[ i ].size;
            glm::vec3& sizeB = componentManager->boxCollider[ j ].size;

            if ( BoxIntersect( posA, posB, sizeA, sizeB ) )
            {
                DEBUG_PRINT( "Entity: %zi hit Entity: %zi \n", i, j );
            }

        }
    }
}

inline bool Solver::BoxIntersect( glm::vec3 posA, glm::vec3 posB, glm::vec3 sizeA, glm::vec3 sizeB )
{
    //Bounding box min and max for A
    float aMaxX = posA.x + sizeA.x;
    float aMinX = posA.x - sizeA.x;
    float aMaxY = posA.y + sizeA.y;
    float aMinY = posA.y - sizeA.y;
    float aMaxZ = posA.z + sizeA.z;
    float aMinZ = posA.z - sizeA.z;

    //Bounding box min and max for B
    float bMaxX = posB.x + sizeB.x;
    float bMinX = posB.x - sizeB.x;
    float bMaxY = posB.y + sizeB.y;
    float bMinY = posB.y - sizeB.y;
    float bMaxZ = posB.z + sizeB.z;
    float bMinZ = posB.z - sizeB.z;

    //check for intersection
    return ( aMinX <= bMaxX && aMaxX >= bMinX ) &&
        ( aMinY <= bMaxY && aMaxY >= bMinY ) &&
        ( aMinZ <= bMaxZ && aMaxZ >= bMinZ );
}

void Solver::Integrate( void* args, int index )
{
    float dt = *( float* ) ( args );
    
    //semi implicit euler 
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& velocity = componentManager->bodyProperties[ i ].velocity;
        glm::vec3& position = componentManager->transform[ i ].pos;

        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = { .0f, .0f, .0f };
    }
    jobManager->AddJob( this, &Physics::Solver::ModelToWorld, nullptr, 0 );

}

void Solver::AccumlateForces( void* args, int index )
{
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& force = componentManager->bodyProperties[ i ].force;
        float& mass = componentManager->bodyProperties[ i ].mass;
        acceleration += force / mass;
    }
    jobManager->AddJob( this, &Physics::Solver::Integrate, args, 0 );

}

void Solver::ModelToWorld( void* args, int index )
{
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        glm::mat4& transformMatrix = componentManager->transform[ i ].transformMatrix;
        glm::vec3& pos = componentManager->transform[ i ].pos;
        glm::vec3& rotationAxis = componentManager->transform[ i ].rot;
        float& rotationAngle = componentManager->transform[ i ].angle;
        glm::vec3& scale = componentManager->transform[ i ].scale;

        transformMatrix =
            glm::translate( pos ) *
            glm::rotate( rotationAngle, rotationAxis ) *
            glm::scale( scale );
    }
}



