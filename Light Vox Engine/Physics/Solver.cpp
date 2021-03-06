#include "Solver.h"

using namespace Physics;

Solver::Solver()
{
    componentManager = ECS::ComponentManager::GetInstance();
    jobManager = Jobs::JobManager::GetInstance();
    rigidbody = new Rigidbody();
    contactSolver = new ContactSolver( 1 );
    debugRenderer = Graphics::DebugRenderer::GetInstance();

    int count = jobManager->GetAmountOfSupportedThreads();
    physicsJobsArgs = std::vector<PhysicsArguments *>();
    physicsJobsArgs.reserve( count );
    promises.reserve( count );
    futures.reserve( count );

    int increment = LV_MAX_INSTANCE_COUNT / count;
    for ( size_t i = 0; i < count; ++i )
    {
        physicsJobsArgs.push_back( new PhysicsArguments() );
        physicsJobsArgs[ i ]->StartElem = increment * i;
        if ( i == count - 1 )
            physicsJobsArgs[ i ]->EndElm = LV_MAX_INSTANCE_COUNT;
        else physicsJobsArgs[ i ]->EndElm = increment * ( i + 1 );

        futures.push_back( std::future<void>() );
        promises.push_back( std::promise<void>() );

        //printf( "[%d] is from %d to %d\n", i, physicsJobsArgs[ i ]->StartElem, physicsJobsArgs[ i ]->EndElm );
    }
}

Solver::~Solver()
{
    jobManager = nullptr;
    componentManager = nullptr;

    if ( rigidbody != nullptr )
    {
        delete rigidbody;
        rigidbody = nullptr;
    }

    if ( contactSolver != nullptr )
    {
        delete contactSolver;
        contactSolver = nullptr;
    }

    for ( size_t i = 0; i < physicsJobsArgs.size(); ++i )
        delete physicsJobsArgs[ i ];
}

void Solver::Update( float dt )
{
    for ( size_t i = 0; i < physicsJobsArgs.size(); ++i )
    {
        promises[ i ] = std::promise<void>();
        physicsJobsArgs[ i ]->DeltaTime = dt;
        futures[ i ] = promises[ i ].get_future();
        physicsJobsArgs[ i ]->jobPromise = &( promises[ i ] );
        jobManager->AddJob( this, &Physics::Solver::AccumlateForces, (void*) ( physicsJobsArgs[ i ] ), 0 );
    }

    for ( size_t i = 0; i < physicsJobsArgs.size(); ++i )
        futures[ i ].wait();
}

void Solver::AccumlateForces( void* args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& force = componentManager->bodyProperties[ i ].force;
        float& mass = componentManager->bodyProperties[ i ].mass;
        acceleration += force / mass;
    }

    jobManager->AddJob( this, &Physics::Solver::AccumlateTorque, args, 0 );
}

void Physics::Solver::AccumlateTorque( void * args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::vec3& angularAccel = componentManager->bodyProperties[ i ].angularAcceleration;
        glm::vec3& torque = componentManager->bodyProperties[ i ].torque;
        glm::mat3& invInertiaTensor = componentManager->bodyProperties[ i ].inertiaTensor;
        angularAccel += invInertiaTensor * torque;
    }
    jobManager->AddJob( this, &Physics::Solver::Integrate, args, 0 );
}

void Solver::Integrate( void* args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );

    float dt = myArgs->DeltaTime;

    //semi implicit euler 
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        if ( !componentManager->bodyProperties[ i ].isAwake )
            continue;

        //movement
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& velocity = componentManager->bodyProperties[ i ].velocity;
        glm::vec3& position = componentManager->transform[ i ].pos;
        glm::vec3& force = componentManager->bodyProperties[ i ].force;
        //rotation
        glm::vec3& angularAccel = componentManager->bodyProperties[ i ].angularAcceleration;
        glm::vec3& rot = componentManager->transform[ i ].rot;
        glm::quat& orientation = componentManager->transform[ i ].orientation;
        glm::vec3& torque = componentManager->bodyProperties[ i ].torque;

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
            rot.z * dt );

        q = q * orientation;

        orientation += ( q * .5f );

        //clear torque and forces
        force = { .0f, .0f, .0f };
        torque = { .0f, .0f, .0f };
    }

    jobManager->AddJob( this, &Physics::Solver::SetColliderData, args, 0 );
}

void Physics::Solver::SetColliderData( void * args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );

    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        rigidbody->CalcHalfSize( i );
    }
    assert( myArgs->jobPromise != nullptr );

    jobManager->AddJob( this, &Physics::Solver::Collide, args, 0 );
}

void Solver::Collide( void* args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );

    float dt = myArgs->DeltaTime;

    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        for ( size_t j = myArgs->StartElem; j < myArgs->EndElm; ++j )
        {
            if ( i == j )
                continue;

            rigidbody->CollideBoxBox( i, j );
        }
    }
    jobManager->AddJob( this, &Physics::Solver::ResolveCollision, args, 0 );
}


void Physics::Solver::ResolveCollision( void * args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );

    float dt = myArgs->DeltaTime;

    size_t contactsFound = componentManager->GetContactsFound();
    contactSolver->SetIterations( contactsFound * 4 );
    contactSolver->ResolveContacts( componentManager->contacts, contactsFound, dt );
    componentManager->ClearContactsFound();

    jobManager->AddJob( this, &Physics::Solver::ModelToWorld, args, 0 );
}

void Solver::ModelToWorld( void* args, int index )
{
    PhysicsArguments* myArgs = static_cast<PhysicsArguments*>( args );
    assert( myArgs != nullptr );

    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::mat4& transformMatrix = componentManager->transformMatrix[ i ].transformMatrix;
        glm::vec3& pos = componentManager->transform[ i ].pos;
        glm::quat& orientation = componentManager->transform[ i ].orientation;
        orientation = glm::normalize( orientation );

        transformMatrix = glm::translate( glm::mat4( 1.0f ), pos ) *  glm::toMat4( orientation );
        transformMatrix = glm::transpose( transformMatrix );
    }

    assert( myArgs->jobPromise != nullptr );

    myArgs->jobPromise->set_value();
}
