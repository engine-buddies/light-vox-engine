#include "Solver.h"

using namespace Physics;

ECS::ComponentManager*      Solver::componentManager = nullptr;
Rigidbody*                  Solver::rigidbody = nullptr;
ContactSolver*              Solver::contactSolver = nullptr;
Graphics::DebugRenderer*    Solver::debugRenderer = nullptr;

Solver::Solver()
{
    componentManager = ECS::ComponentManager::GetInstance();
    rigidbody = new Rigidbody();
    contactSolver = new ContactSolver( 1 );
    debugRenderer = Graphics::DebugRenderer::GetInstance();

    const unsigned int count = JobManager::Instance.GetAmountOfSupportedThreads();
    physicsJobsArgs = std::vector<PhysicsArguments *>();
    physicsJobsArgs.reserve( count );

    int increment = LV_MAX_INSTANCE_COUNT / count;
    for ( size_t i = 0; i < count; ++i )
    {
        physicsJobsArgs.push_back( new PhysicsArguments() );
        physicsJobsArgs[ i ]->StartElem = increment * i;
        if ( i == count - 1 )
        {
            physicsJobsArgs[ i ]->EndElm = LV_MAX_INSTANCE_COUNT;
        }
        else
        {
            physicsJobsArgs[ i ]->EndElm = increment * ( i + 1 );
        }
    }
}

Solver::~Solver()
{
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
    Job* root = JobManager::Instance.CreateJob( 
        &AccumlateForces,
        physicsJobsArgs[ 0 ],
        sizeof( physicsJobsArgs[ 0 ] )
    );

    for ( size_t i = 1; i < physicsJobsArgs.size(); ++i )
    {
        physicsJobsArgs[ i ]->DeltaTime = dt;
        Job* child = JobManager::Instance.CreateJobAsChild(
            root, 
            &AccumlateForces,
            physicsJobsArgs[ i ],
            sizeof( physicsJobsArgs[ i ] )
        );
        JobManager::Instance.Run( child );
    }
    JobManager::Instance.Run( root );
    JobManager::Instance.Wait( root );

}

void Solver::AccumlateForces( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
    assert( myArgs != nullptr );
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& force = componentManager->bodyProperties[ i ].force;
        float& mass = componentManager->bodyProperties[ i ].mass;
        acceleration += force / mass;
    }

    AccumlateTorque( aJob, aData );

    //jobManager->AddJob( this, &Physics::Solver::AccumlateTorque, args, 0 );
}

void Physics::Solver::AccumlateTorque( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
    assert( myArgs != nullptr );
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::vec3& angularAccel = componentManager->bodyProperties[ i ].angularAcceleration;
        glm::vec3& torque = componentManager->bodyProperties[ i ].torque;
        glm::mat3& invInertiaTensor = componentManager->bodyProperties[ i ].inertiaTensor;
        angularAccel += invInertiaTensor * torque;
    }

    Integrate( aJob, aData );
    //jobManager->AddJob( this, &Physics::Solver::Integrate, args, 0 );
}

void Solver::Integrate( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
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
    SetColliderData( aJob, aData );
    //jobManager->AddJob( this, &Physics::Solver::SetColliderData, args, 0 );
}

void Physics::Solver::SetColliderData( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
    assert( myArgs != nullptr );

    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        rigidbody->CalcHalfSize( i );
    }
    Collide( aJob, aData );
    //jobManager->AddJob( this, &Physics::Solver::Collide, args, 0 );
}

void Physics::Solver::Collide( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
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
    ResolveCollision( aJob, aData );
    //jobManager->AddJob( this, &Physics::Solver::ResolveCollision, args, 0 );
}


void Physics::Solver::ResolveCollision( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
    assert( myArgs != nullptr );

    float dt = myArgs->DeltaTime;

    size_t contactsFound = componentManager->GetContactsFound();
    contactSolver->SetIterations( contactsFound * 4 );
    contactSolver->ResolveContacts( componentManager->contacts, contactsFound, dt );
    componentManager->ClearContactsFound();

    ModelToWorld( aJob, aData );
    //jobManager->AddJob( this, &Physics::Solver::ModelToWorld, args, 0 );
}

void Solver::ModelToWorld( Job* aJob, const void* aData )
{
    const PhysicsArguments* myArgs = static_cast< const PhysicsArguments* >( aData );
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

    //assert( myArgs->jobPromise != nullptr );

    //myArgs->jobPromise->set_value();
}
