#include "Solver.h"

using namespace Physics;

Solver::Solver()
{
    gravity = { .0f, .0f, .0f };
    componentManager = ECS::ComponentManager::GetInstance();
    jobManager = Jobs::JobManager::GetInstance();

    a_argument = new PhysicsArguments();
    a_argument->StartElem = 0;
    a_argument->EndElm = ( LV_MAX_INSTANCE_COUNT  / 2);

    b_argument = new PhysicsArguments();
    b_argument->StartElem = ( LV_MAX_INSTANCE_COUNT / 2 );
    b_argument->EndElm = LV_MAX_INSTANCE_COUNT;
}

Solver::~Solver()
{
    jobManager = nullptr;

    if ( a_argument != nullptr )
    {
        delete a_argument;
        a_argument = nullptr;
    }

    if ( b_argument != nullptr )
    {
        delete b_argument;
        b_argument = nullptr;
    }
}

void Solver::Update( float dt )
{
    //static float deltaTime = 0.016f;
    a_argument->DeltaTime = dt;
    b_argument->DeltaTime = dt;

    std::promise<void> aPromise;
    std::future<void> aFuture = aPromise.get_future();
    a_argument->jobPromise = &aPromise;

    std::promise<void> bPromise;
    std::future<void> bFuture = bPromise.get_future();
    b_argument->jobPromise = &bPromise;


    jobManager->AddJob( this, &Physics::Solver::AccumlateForces, ( void* ) ( a_argument ), 0 );
    jobManager->AddJob( this, &Physics::Solver::AccumlateForces, ( void* ) ( b_argument ), 0 );

    aFuture.wait();
    bFuture.wait();

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
    PhysicsArguments* myArgs = ( PhysicsArguments* ) ( args );
    float dt = myArgs->DeltaTime;

    //semi implicit euler 
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& velocity = componentManager->bodyProperties[ i ].velocity;
        glm::vec3& position = componentManager->transform[ i ].pos;

        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = { .0f, .0f, .0f };
    }

    jobManager->AddJob( this, &Physics::Solver::ModelToWorld, args, 0 );
}

void Solver::AccumlateForces( void* args, int index )
{
    PhysicsArguments* myArgs = ( PhysicsArguments* ) ( args );

    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
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
    PhysicsArguments* myArgs = ( PhysicsArguments* ) ( args );

    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
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

    if ( myArgs->jobPromise != nullptr )
    {
        myArgs->jobPromise->set_value();
    }
}



