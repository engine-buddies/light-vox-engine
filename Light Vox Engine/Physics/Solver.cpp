#include "Solver.h"

using namespace Physics;

Solver::Solver()
{
    componentManager = ECS::ComponentManager::GetInstance();
    jobManager = Jobs::JobManager::GetInstance();
    rigidbody = new Rigidbody();
    contactSolver = new ContactSolver(1);
    debugRenderer = Graphics::DebugRenderer::GetInstance();

    a_argument = new PhysicsArguments();
    a_argument->StartElem = 0;
    a_argument->EndElm = ( LV_MAX_INSTANCE_COUNT  / 2 );

    b_argument = new PhysicsArguments();
    b_argument->StartElem = ( LV_MAX_INSTANCE_COUNT / 2 );
    b_argument->EndElm = LV_MAX_INSTANCE_COUNT;

}

Solver::~Solver()
{
    jobManager = nullptr;
    componentManager = nullptr;

    if (rigidbody != nullptr)
    {
        delete rigidbody;
        rigidbody = nullptr;
    }

    if (contactSolver != nullptr)
    {
        delete contactSolver;
        contactSolver = nullptr;
    }

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

//SINGLE THREADED
void Solver::Update(float dt)
{
    AccumlateForces();
    AccumlateTorque();
    Integrate(dt);
    SetColliderData();
    Collide();
    ResolveContacts(dt);
    ModelToWorld();
}



void Solver::Collide()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        for (size_t j = 0; j < LV_MAX_INSTANCE_COUNT; ++j)
        {
            if (i == j)
                continue;

            rigidbody->CollideBoxBox(i, j);
        }
    }
}

void Solver::AccumlateForces()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        glm::vec3& acceleration = componentManager->bodyProperties[i].acceleration;
        glm::vec3& force = componentManager->bodyProperties[i].force;
        float& mass = componentManager->bodyProperties[i].mass;
        acceleration += force / mass;
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



void Solver::Integrate(float dt)
{
    //semi implicit euler 
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        if (!componentManager->bodyProperties[i].isAwake)
            continue;

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

void Solver::ModelToWorld()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        glm::mat4& transformMatrix = componentManager->transformMatrix[i].transformMatrix;
        glm::vec3& pos = componentManager->transform[i].pos;
        glm::quat& orientation = componentManager->transform[i].orientation;
        orientation = glm::normalize(orientation);

        transformMatrix = glm::translate(glm::mat4(1.0f), pos) *  glm::toMat4(orientation);
        transformMatrix = glm::transpose(transformMatrix);
    }
}

void Physics::Solver::SetColliderData()
{
    for (size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i)
    {
        rigidbody->CalcHalfSize(i);
    }
}

void Physics::Solver::ResolveContacts(float dt)
{
    contactSolver->SetIterations(componentManager->contactsFound * 4);
    contactSolver->ResolveContacts(componentManager->contacts, componentManager->contactsFound, dt);
    componentManager->contactsFound = 0;
}

