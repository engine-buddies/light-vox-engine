#include "Physics.h"


Physics::Physics()
{
    gravity = { .0f, .0f, .0f };
    componentManager = ComponentManager::GetInstance();
}

Physics::~Physics()
{
}

void Physics::Update( float dt )
{
    Collide();
    AccumlateForces();
    Integrate( dt );
    ModelToWorld();
    //SatisfyConstraints();
}

void Physics::Collide()
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
#ifdef DEBUG
                printf( "Entity: %i hit Entity: %i \n", i, j );
#endif // DEBUG

            }

        }
    }
}

inline bool Physics::BoxIntersect( glm::vec3 posA, glm::vec3 posB, glm::vec3 sizeA, glm::vec3 sizeB )
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

void Physics::Move( glm::vec3 pos, UINT index )
{
    componentManager->transform[ index ].pos = pos;
}

void Physics::RotateAxisAngle( glm::vec3 rotationAxis, float angle, UINT index )
{
    componentManager->transform[ index ].rot = rotationAxis;
    componentManager->transform[ index ].angle = angle;
}

void Physics::Integrate( float dt )
{
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
}

void Physics::AccumlateForces()
{
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& force = componentManager->bodyProperties[ i ].force;
        float& mass = componentManager->bodyProperties[ i ].mass;
        acceleration += force / mass;
    }
}

void Physics::ModelToWorld()
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



