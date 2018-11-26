#include "ContactSolver.h"

using namespace Physics;



namespace
{
    //Constructs an arbitary orthonormal basis for the contact
    inline void CalculateContactBasis(Contacts* c)
    {
        glm::vec3 contactTangent[2];
        glm::vec3 contactNormal = c->contactNormal;


        if (glm::abs(contactNormal.x) > glm::abs(contactNormal.y))
        {
            //scaling factor to ensure the results are normalized
            const float s = (float)(1.0f / contactNormal.z * contactNormal.z +
                contactNormal.x * contactNormal.x);

            //the new x axis is at right angles to the world Y axis
            contactTangent[0].x = contactNormal.z * s;
            contactTangent[0].y = 0;
            contactTangent[0].z = -contactNormal.x * s;

            //the new y axis is at right angles to the new x and z axes
            contactTangent[1].x = contactNormal.y*contactTangent[0].x;
            contactTangent[1].y = contactNormal.z*contactTangent[0].x -
                contactNormal.x*contactTangent[0].z;
            contactTangent[1].z = -contactNormal.y*contactTangent[0].x;

        }
        else
        {
            //scaling factor to ensure the results are normalized
            const float s = (float)(1.0f / contactNormal.z * contactNormal.z +
                contactNormal.y * contactNormal.y);

            //the new x axis is at right angles to the world Y axis
            contactTangent[0].x = 0;
            contactTangent[0].y = contactNormal.z * s;;
            contactTangent[0].z = -contactNormal.y * s;

            //the new y axis is at right angles to the new x and z axes
            contactTangent[1].x = contactNormal.y*contactTangent[0].z -
                contactNormal.z*contactTangent[0].y;
            contactTangent[1].y = -contactNormal.x*contactTangent[0].z;
            contactTangent[1].z = contactNormal.x*contactTangent[0].y;
        }

        //make a matrix from the three vectors
        float basis[9] = {
            contactNormal.x, contactNormal.y, contactNormal.z,
            contactTangent[0].x, contactTangent[0].y, contactTangent[0].z,
            contactTangent[1].x, contactTangent[1].y, contactTangent[1].z
        };

        c->contactWorld = glm::make_mat3x3(basis);
    }


    inline void MatchAwakeState(BodyProperties* a, BodyProperties* b)
    {
        if(!b) return;

        bool body0Awake = &a->isAwake;
        bool body1Awake = &b->isAwake;

        if(body0Awake ^ body1Awake)
        {
            if(body0Awake) 
                body0Awake = true;
            else if (body1Awake)
                body1Awake = true;
        }
    }

}

Physics::ContactSolver::ContactSolver(uint16_t itertations, float velEpsilon, float posEpsilon)
{
    componentManager = ECS::ComponentManager::GetInstance();
    positionIterations = itertations;
    velocityIterations = itertations;
    positionEpsilon = posEpsilon;
    velocityEpsilon = velEpsilon;
}

Physics::ContactSolver::~ContactSolver()
{
    componentManager = nullptr;
}

void Physics::ContactSolver::ResolveContacts(Contacts * contacts, uint32_t numContacts, float dt)
{
    //make sure we have something to do 
    if (numContacts == 0) return;
    if (!isValid()) return;

    //prepareContacts

    //adjustPositions

    //adjustVelocities 
}

bool Physics::ContactSolver::isValid()
{
    return (velocityIterations > 0) &&
        (positionIterations > 0) &&
        (velocityEpsilon >= 0.0f) &&
        (positionEpsilon >= 0.0f);
}

glm::vec3 Physics::ContactSolver::CalculateLocalVelocity(uint32_t bodyIndex, float dt)
{
    Transform* thisBody = &componentManager->transform[bodyIndex];
    Contacts* c = &componentManager->contacts[bodyIndex];
    BodyProperties* properties = &componentManager->bodyProperties[bodyIndex];

    //work out the velocity of the contact point
    glm::vec3 velocity =
        glm::cross(thisBody->rot, c->relativeContactPosition[bodyIndex]);

    velocity += properties->velocity;

    //turn the velocity inyo contact cordinates
    glm::vec3 contactVelocity = glm::transpose(c->contactWorld) * velocity;

    //calc. the amount of velocity that is due to force without 
    //reaction
    glm::vec3 accVelocity = properties->acceleration * dt;

    accVelocity = glm::transpose(c->contactWorld) * accVelocity;
    accVelocity.x = 0;
    contactVelocity += accVelocity;

    return contactVelocity;
}

void Physics::ContactSolver::CalculateDesiredDeltaVelocity(float dt, Contacts* c)
{
    const static float velocityLimit = 0.25f;
    BodyProperties* bodyA = &componentManager->bodyProperties[c->bodyPair.a];
    BodyProperties* bodyB = &componentManager->bodyProperties[c->bodyPair.b];

    //calc. the acceleration induced velocity accumalted this frame
    float velocityFromAcc = 0;

    if (bodyA->isAwake)
    {
        velocityFromAcc += 
            glm::dot((bodyA->acceleration * dt), c->contactNormal);
    }

    if (bodyB && bodyB->isAwake)
    {
        velocityFromAcc -= 
            glm::dot((bodyB->acceleration * dt), c->contactNormal);
    }

    //if the velocity is  slow, limit restitution
    float thisRestitution = c->restitution;
    if (glm::abs(c->contactVelocity.x) < velocityLimit);
    {
        thisRestitution = 0.0f;
    }

    c->desiredVelocity =
        -c->contactVelocity.x - thisRestitution * (c->contactVelocity.y - velocityFromAcc);

}

void Physics::ContactSolver::ApplyVelocityChanges(
    glm::vec3 velocityChange[2], 
    glm::vec3 rotationChange[2],
    Contacts* c)
{
    BodyProperties* a  = &componentManager->bodyProperties[c->bodyPair.a];
    BodyProperties* b = &componentManager->bodyProperties[c->bodyPair.b];
    //Convert intertia tensor to world coords.
    glm::mat3x3 inverseInteriaTensor[2];
    inverseInteriaTensor[0] = a->inertiaTensor;
    if (b)
        inverseInteriaTensor[1] = b->inertiaTensor;

    glm::vec3 impulseContact;

    if (c->friction == 0.0f)
    {
        //calcFrcitionlessImpluse
    }
    else
    {
        //calcFritionImpluse 
    }
    glm::vec3 impulse = c->contactWorld * impulseContact;

    //split in the impulse into linear and rotational component
    glm::vec3 impulsiveTorque = glm::cross(c->relativeContactPosition[0], impulse);
    rotationChange[0] = inverseInteriaTensor[0] * impulsiveTorque;
    velocityChange[0] = glm::vec3(0.0f);
    velocityChange[0] += (impulse * a->invMass);

    //apply changes
    a->velocity += velocityChange[0];
    //a->rot
    


}

void Physics::ContactSolver::ApplyPositionChanges(
    glm::vec3 linearChange[2], 
    glm::vec3 angularChange[2],
    Contacts* c)
{
}

void Physics::ContactSolver::CalculateInternals(float dt, Contacts* c)
{
    //calc. a set of axis at the contact point
    CalculateContactBasis(c);

    //store the relative position
    c->relativeContactPosition[0] = c->contactPoint - componentManager->transform[c->bodyPair.a].pos;
    if(c->bodyPair.a)
    {
        c->relativeContactPosition[1] = c->contactPoint - componentManager->transform[c->bodyPair.b].pos;
    }
    
    //find the relative velocity of the bodies at the contact
    c->contactVelocity = CalculateLocalVelocity(c->bodyPair.a, dt);
    if (c->bodyPair.b)
    {
        c->contactVelocity -= CalculateLocalVelocity(c->bodyPair.b, dt);
    }

    //calc. desired velocity change 
    CalculateDesiredDeltaVelocity(dt, c);
}

void Physics::ContactSolver::PrepareContacts(Contacts * contacts, uint32_t numContacts, float dt)
{
    //generate contact velocity and axis information
    Contacts* lastContacts = contacts + numContacts;
    for (Contacts* contact = contacts; contact < lastContacts; contact++)
    {
        //calc. internal (inertia, basis, etc.)
        CalculateInternals(dt, contact);
    }
}

void Physics::ContactSolver::AdjustVelocities(
    Contacts * contacts, 
    uint32_t numContacts, 
    float dt)
{
    glm::vec3 velocityChange[2], rotationChange[2];
    glm::vec3 deltaVel;

    // iteratively handle impacts in order of severity.
    velocityIterationsUsed = 0;
    while(velocityIterationsUsed < velocityIterations)
    {
        float max = velocityEpsilon;
        uint32_t index = numContacts;
        for(size_t i = 0; i < numContacts; ++i)
        {
            if(contacts[i].desiredVelocity > max)
            {
                max = contacts[i].desiredVelocity;
                index = i;
            }
        }

        if(index == numContacts) break;

        BodyProperties* bodyA = &componentManager->bodyProperties[contacts[index].bodyPair.a];
        BodyProperties* bodyB = &componentManager->bodyProperties[contacts[index].bodyPair.b];
        //match the awake state at the contact
        MatchAwakeState(bodyA, bodyB);

        //do the resolution on the contact that came out top
        //applyVelocityChange

        for(size_t i = 0; i < numContacts; ++i)
        {
            for(size_t j = 0; j < 2; ++j) if(contacts[i].bodyPair.b)
            {
                //check for a match with each body
                for(size_t k = 0; k < 2; ++k)
                {
                    deltaVel = velocityChange[k] + 
                        rotationChange[k] * contacts[index].relativeContactPosition[j];

                    //the sign of the change is negative if we're dealing
                    //with the 2nd body
                    contacts[index].contactVelocity += 
                        glm::transpose(contacts[index].contactWorld) * deltaVel *
                            (j ? -1.0f : 1.0f);
                    
                    CalculateDesiredDeltaVelocity(dt, &contacts[index]);
                }
            }
        }
        velocityIterations++;
    }

}

void Physics::ContactSolver::AdjustPositions(
    Contacts * contacts, 
    uint32_t numContacts, 
    float dt)
{
    unsigned i, index;
    glm::vec3 linearChange[2], angularChange[2];
    float max;
    glm::vec3 deltaPosition;

    positionIterationsUsed = 0;


    while(positionIterationsUsed < positionIterations)
    {
        max = positionEpsilon;
        index = numContacts;
        for(size_t i = 0; i < numContacts; ++i)
        {
            if(contacts[i].penetration > max)
            {
                max = contacts[i].penetration;
                index = i;
            }
        }

        if(index == numContacts) break;

        BodyProperties* bodyA = &componentManager->bodyProperties[contacts[index].bodyPair.a];
        BodyProperties* bodyB = &componentManager->bodyProperties[contacts[index].bodyPair.b];
        //match the awake state at the contact
        MatchAwakeState(bodyA, bodyB);

        //Resolve the penetration 
        //ApplyPositionChange

        // Again this action may have changed the penetration of other
        // bodies, so we update contacts.
        for(size_t i = 0; i < numContacts; ++i)
        {
            for(size_t j = 0; j < 2; ++j) if(contacts[i].bodyPair.b)
            {
                //check for a match with each body
                for(size_t k = 0; k < 2; ++k)
                {
                    deltaPosition = linearChange[k] + 
                        angularChange[k] * contacts[index].relativeContactPosition[j];

                    //the sign of the change is negative if we're dealing
                    //with the 2nd body
                    contacts[index].penetration += 
                        glm::dot(deltaPosition, contacts[index].contactNormal) *
                            (j ? -1.0f : 1.0f);
                    
                }
            }
        }
        positionIterationsUsed++;
    }

}
