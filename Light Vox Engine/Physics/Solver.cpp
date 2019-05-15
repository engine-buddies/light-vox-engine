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
	physicsJobsArgs[ 0 ]->DeltaTime = dt;
	Job* root = JobManager::Instance.CreateJob( &AccumlateForces, &physicsJobsArgs[ 0 ], sizeof( PhysicsArguments * * ) );

	for( unsigned int i = 1; i < physicsJobsArgs.size(); ++i )
	{
		physicsJobsArgs[ i ]->DeltaTime = dt;
		Job* child = JobManager::Instance.CreateJobAsChild( root, &AccumlateForces, &physicsJobsArgs[ i ], sizeof( PhysicsArguments * * ) );
		JobManager::Instance.Run( child );
	}

	JobManager::Instance.Run( root );
	JobManager::Instance.Wait( root );
}

void Solver::AccumlateForces( Job* aJob, const void* aData )
{
	PhysicsArguments* myArgs = nullptr;
	memcpy( &myArgs, aData, sizeof( PhysicsArguments * * ) );

    assert( myArgs != nullptr );
    for ( size_t i = myArgs->StartElem; i < myArgs->EndElm; ++i )
    {
        glm::vec3& acceleration = componentManager->bodyProperties[ i ].acceleration;
        glm::vec3& force = componentManager->bodyProperties[ i ].force;
        float& mass = componentManager->bodyProperties[ i ].mass;
        acceleration += force / mass;
    }

    AccumlateTorque( myArgs );
}

void Solver::AccumlateTorque( PhysicsArguments* aData )
{
	assert( aData != nullptr );
    for ( size_t i = aData->StartElem; i < aData->EndElm; ++i )
    {
        glm::vec3& angularAccel = componentManager->bodyProperties[ i ].angularAcceleration;
        glm::vec3& torque = componentManager->bodyProperties[ i ].torque;
        glm::mat3& invInertiaTensor = componentManager->bodyProperties[ i ].inertiaTensor;
        angularAccel += invInertiaTensor * torque;
    }

    Integrate( aData );
}

void Solver::Integrate( PhysicsArguments* aData )
{
	assert( aData != nullptr );

    float dt = aData->DeltaTime;

    //semi implicit euler 
    for ( size_t i = aData->StartElem; i < aData->EndElm; ++i )
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

        // Angular velocity formula for quaternions 
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
    SetColliderData( aData );
}

void Physics::Solver::SetColliderData( PhysicsArguments* aData )
{
	assert( aData != nullptr );

    for ( size_t i = aData->StartElem; i < aData->EndElm; ++i )
    {
        rigidbody->CalcHalfSize( i );
    }
    Collide( aData );
}

void Physics::Solver::Collide( PhysicsArguments* aData )
{
    assert( aData != nullptr );

    float dt = aData->DeltaTime;

    for ( size_t i = aData->StartElem; i < aData->EndElm; ++i )
    {
        for ( size_t j = aData->StartElem; j < aData->EndElm; ++j )
        {
            if ( i == j )
                continue;

            rigidbody->CollideBoxBox( i, j );
        }
    }
    ResolveCollision( aData );
}

void Physics::Solver::ResolveCollision( PhysicsArguments* aData )
{
	assert( aData != nullptr );

    float dt = aData->DeltaTime;

    size_t contactsFound = componentManager->GetContactsFound();
    contactSolver->SetIterations( contactsFound * 4 );
    contactSolver->ResolveContacts( componentManager->contacts, contactsFound, dt );
    componentManager->ClearContactsFound();

    ModelToWorld( aData );
}

void Solver::ModelToWorld( PhysicsArguments* aData )
{
    assert( aData != nullptr );

    for ( size_t i = aData->StartElem; i < aData->EndElm; ++i )
    {
        glm::mat4& transformMatrix = componentManager->transformMatrix[ i ].transformMatrix;
        glm::vec3& pos = componentManager->transform[ i ].pos;
        glm::quat& orientation = componentManager->transform[ i ].orientation;
        orientation = glm::normalize( orientation );

        transformMatrix = glm::translate( glm::mat4( 1.0f ), pos ) *  glm::toMat4( orientation );
        transformMatrix = glm::transpose( transformMatrix );
    }
}
