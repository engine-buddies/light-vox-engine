#include "ContactSolver.h"

using namespace Physics;
using namespace EntityComponents;

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

        //create basis matrix from the three vectors
        glm::mat3 contactToWorld;
        contactToWorld[0] = contactNormal;
        contactToWorld[1] = contactTangent[0];
        contactToWorld[2] = contactTangent[1];
        c->contactWorld = contactToWorld;
    }


    inline void MatchAwakeState(BodyProperties* a, BodyProperties* b)
    {
        if (!b) return;

        bool body0Awake = &a->isAwake;
        bool body1Awake = &b->isAwake;

        if (body0Awake ^ body1Awake)
        {
            if (body0Awake)
                body0Awake = true;
            else if (body1Awake)
                body1Awake = true;
        }
    }

    inline glm::vec3 calcFrictionlessImpulse(
        glm::mat3* inverseInertiaTensor,
        float invMassA,
        float invMassB,
        Contacts* c)
    {
        glm::vec3 impulseContact;

        //Build a vector that shows the change in velocity in 
        //world space 

        glm::vec3 deltaVelWorld =
            glm::cross(c->relativeContactPosition[0], c->contactNormal);
        deltaVelWorld = inverseInertiaTensor[0] * deltaVelWorld;
        deltaVelWorld = glm::cross(deltaVelWorld, c->relativeContactPosition[0]);

        //work out the change in velocity in contact coordinates
        float deltaVelocity = glm::dot(deltaVelWorld, c->contactNormal);

        deltaVelocity += invMassA;

        deltaVelWorld = glm::cross(c->relativeContactPosition[1], c->contactNormal);
        deltaVelWorld = inverseInertiaTensor[1] * deltaVelWorld;
        deltaVelWorld = glm::cross(deltaVelWorld, c->relativeContactPosition[1]);

        // Add the change in velocity due to rotation
        deltaVelocity += glm::dot(deltaVelWorld, c->contactNormal);

        // Add the change in velocity due to linear motion
        deltaVelocity += invMassB;

        // Calculate the required size of the impulse
        impulseContact.x = c->desiredVelocity / deltaVelocity;
        impulseContact.y = 0;
        impulseContact.z = 0;
        return impulseContact;
    }
}

Physics::ContactSolver::ContactSolver(uint16_t iterations, float velEpsilon, float posEpsilon)
{
    componentManager = ECS::ComponentManager::GetInstance();
    positionIterations = iterations;
    velocityIterations = iterations;
    positionEpsilon = posEpsilon;
    velocityEpsilon = velEpsilon;
}

Physics::ContactSolver::~ContactSolver()
{
    componentManager = nullptr;
}

void Physics::ContactSolver::ResolveContacts(Contacts * contacts, size_t numContacts, float dt)
{
    //make sure we have something to do 
    if (numContacts == 0 || numContacts > 0xfffff) return;
    if (!isValid()) return;

    // Prepare the contacts for processing
    PrepareContacts(contacts, numContacts, dt);

    // Solve interpenetration 
    AdjustPositions(contacts, numContacts, dt);

    // Resolve velocity 
    AdjustVelocities(contacts, numContacts, dt);
}

void Physics::ContactSolver::SetIterations(size_t iterations)
{
    positionIterations = iterations;
    velocityIterations = iterations;
}

bool Physics::ContactSolver::isValid()
{
    return (velocityIterations > 0) &&
        (positionIterations > 0) &&
        (velocityEpsilon >= 0.0f) &&
        (positionEpsilon >= 0.0f);
}

glm::vec3 Physics::ContactSolver::CalculateLocalVelocity(Contacts* c, uint32_t bodyIndex, uint16_t bodyPair, float dt)
{
    Transform* thisBody = &componentManager->transform[bodyIndex];
    BodyProperties* properties = &componentManager->bodyProperties[bodyIndex];

    //work out the velocity of the contact point
    glm::vec3 velocity =
        glm::cross(thisBody->rot, c->relativeContactPosition[bodyPair]);

    velocity += properties->velocity;

    //turn the velocity into contact cordinates
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
    float velocityLimit = 0.25f;
    BodyProperties* bodyA = &componentManager->bodyProperties[c->bodyPair.a];
    BodyProperties* bodyB = &componentManager->bodyProperties[c->bodyPair.b];

    //calc. the acceleration induced velocity accumalted this frame
    float velocityFromAcc = 0;

    if (bodyA->isAwake)
    {
        velocityFromAcc +=
            glm::dot((bodyA->acceleration * dt), c->contactNormal);
    }

    if (bodyB->isAwake)
    {
        velocityFromAcc -=
            glm::dot((bodyB->acceleration * dt), c->contactNormal);
    }

    //if the velocity is  slow, limit restitution
    float thisRestitution = c->restitution;
    if (glm::abs(c->contactVelocity.x) < velocityLimit)
    {
        thisRestitution = 0.0f;
    }

    c->desiredVelocity =
        -c->contactVelocity.x - thisRestitution * (c->contactVelocity.y - velocityFromAcc);

}

void Physics::ContactSolver::CalculateInternals(float dt, Contacts* c)
{
    //calc. a set of axis at the contact point
    CalculateContactBasis(c);

    //store the relative position
    c->relativeContactPosition[0] = c->contactPoint - componentManager->transform[c->bodyPair.a].pos;
    c->relativeContactPosition[1] = c->contactPoint - componentManager->transform[c->bodyPair.b].pos;

    //find the relative velocity of the bodies at the contact
    c->contactVelocity = CalculateLocalVelocity(c, c->bodyPair.a, 0, dt);
    c->contactVelocity -= CalculateLocalVelocity(c, c->bodyPair.b, 1, dt);

    //calc. desired velocity change 
    CalculateDesiredDeltaVelocity(dt, c);
}

void Physics::ContactSolver::ApplyVelocityChange(
    glm::vec3 velocityChange[2],
    glm::vec3 rotationChange[2],
    Contacts* c)
{
    //Body A
    BodyProperties* a = &componentManager->bodyProperties[c->bodyPair.a];
    Transform* aTransform = &componentManager->transform[c->bodyPair.a];
    //Body B
    BodyProperties* b = &componentManager->bodyProperties[c->bodyPair.b];
    Transform* bTransform = &componentManager->transform[c->bodyPair.b];

    //Convert intertia tensor to world coords.
    glm::mat3x3 inverseInteriaTensor[2];
    inverseInteriaTensor[0] = a->inertiaTensor;
    inverseInteriaTensor[1] = b->inertiaTensor;

    glm::vec3 impulseContact;
    impulseContact = calcFrictionlessImpulse(inverseInteriaTensor, a->invMass, b->invMass, c);
    glm::vec3 impulse = c->contactWorld * impulseContact;

    //split in the impulse into linear and rotational component
    glm::vec3 impulsiveTorque = glm::cross(c->relativeContactPosition[0], impulse);
    rotationChange[0] = inverseInteriaTensor[0] * impulsiveTorque;
    velocityChange[0] = glm::vec3(0.0f);
    velocityChange[0] += (impulse * a->invMass);

    //apply changes
    a->velocity += velocityChange[0];
    aTransform->rot += rotationChange[0];

    //Apply changes to body B
    impulsiveTorque = glm::cross(c->relativeContactPosition[1], impulse);
    rotationChange[1] = inverseInteriaTensor[0] * impulsiveTorque;
    velocityChange[1] = glm::vec3(0.0f);
    velocityChange[1] += (impulse * -b->invMass);

    //apply changes
    b->velocity += velocityChange[1];
    bTransform->rot += rotationChange[1];

}

void Physics::ContactSolver::ApplyPositionChange(
    glm::vec3 linearChange[2],
    glm::vec3 angularChange[2],
    Contacts* c,
    float penetration)
{
	ContactBodies contactBodies[2];
	contactBodies[0].bodyProps = componentManager->bodyProperties[c->bodyPair.a];
	contactBodies[0].transform = componentManager->transform[c->bodyPair.a];
	contactBodies[1].bodyProps = componentManager->bodyProperties[c->bodyPair.b];
	contactBodies[1].transform = componentManager->transform[c->bodyPair.b];

	const float angularLimit = (float)2.0f;
	float angularMove[2];
	float linearMove[2];

	float totalInertia = 0;
	float linearInertia[2];
	float angularInertia[2];

	//work out the inertia of each object in the direction
	//of the contact normal, due to angular inertia only.
	//calc. total intertia before moving on
	for (size_t i = 0; i < 2; ++i)
	{
		glm::mat3x3 inverseInertiaTensor = contactBodies[i].bodyProps.inertiaTensor;
		glm::vec3 angularInertiaWorld = glm::cross(c->relativeContactPosition[i], c->contactNormal);
		angularInertiaWorld = inverseInertiaTensor * angularInertiaWorld;
		angularInertiaWorld = glm::cross(angularInertiaWorld, c->relativeContactPosition[i]);
		angularInertia[i] = glm::dot(angularInertiaWorld, c->contactNormal);

		linearInertia[i] = contactBodies[i].bodyProps.invMass;
		totalInertia += linearInertia[i] + angularInertia[i];
	}


	for (size_t i = 0; i < 2; ++i)
	{
		uint32_t bodyIndex = (i == 0) ? c->bodyPair.a : c->bodyPair.b;

		//the linear and angular movements required are in proportion to the 
		//inverse inertias
		float sign = (i == 0) ? -1.0f : 1.0f;
		angularMove[i] =
			sign * penetration * (angularInertia[i] / totalInertia);
		linearMove[i] =
			sign * penetration * (linearInertia[i] / totalInertia);

		//avoid angular projection that are too great (when mass is large
		//but inertia tensor is small) limit the angular move
		glm::vec3 projection = c->relativeContactPosition[i];
		projection +=
			(c->contactNormal * (-glm::dot(c->relativeContactPosition[i], c->contactNormal)));

		//use small angle aprox. for sin of the angle
		float maxMagnitude = angularLimit * glm::length(projection);

		if (angularMove[i] < -maxMagnitude)
		{
			float totalMove = angularMove[i] + linearMove[i];
			angularMove[i] = -maxMagnitude;
			linearMove[i] = totalMove - angularMove[i];
		}
		else if (angularMove[i] > maxMagnitude)
		{
			float totalMove = angularMove[i] + linearMove[i];
			angularMove[i] = maxMagnitude;
			linearMove[i] = totalMove - angularMove[i];
		}

		//we have linear amount of movement required by turning 
		//rigid body. we now need to calc. desired rotation
		if (angularMove[i] == 0)
		{
			angularChange[i] = glm::vec3(0);
		}
		else
		{
			// Work out the direction we'd like to rotate in.
			glm::vec3 targetAngularDirection =
				glm::cross(c->relativeContactPosition[i], c->contactNormal);

			glm::mat3 inverseInertiaTensor;
			inverseInertiaTensor = contactBodies[i].bodyProps.inertiaTensor;

			// Work out the direction we'd need to rotate to achieve that
			angularChange[i] =
				(inverseInertiaTensor * targetAngularDirection) *
				(angularMove[i] / angularInertia[i]);
		}

		// Velocity change is easier - it is just the linear movement
		// along the contact normal.
		linearChange[i] = c->contactNormal * linearMove[i];

		// Now we can start to apply the values we've calculated.
		// Apply the linear movement
		glm::vec3 pos;
		pos = contactBodies[i].transform.pos;
		pos += c->contactNormal * linearMove[i];
		componentManager->transform[bodyIndex].pos = pos;

		// And the change in orientation
		glm::quat q;
		glm::quat rot;
		rot = glm::quat(
			0,
			angularChange[i].x,
			angularChange[i].y,
			angularChange[i].z);


		q = (rot * .5f);
		componentManager->transform[bodyIndex].orientation += q;
    }
}

void Physics::ContactSolver::PrepareContacts(Contacts * contacts, size_t numContacts, float dt)
{
    //generate contact velocity and axis information
    for (size_t i = 0; i < numContacts; ++i)
    {
        CalculateInternals(dt, &contacts[i]);
    }
}

void Physics::ContactSolver::AdjustVelocities(
    Contacts * contacts,
    size_t numContacts,
    float dt)
{
    glm::vec3 velocityChange[2], rotationChange[2];
    glm::vec3 deltaVel;

    // iteratively handle impacts in order of severity.
    velocityIterationsUsed = 0;
    while (velocityIterationsUsed < velocityIterations)
    {
        float max = velocityEpsilon;
        size_t index = numContacts;
        for (size_t i = 0; i < numContacts; ++i)
        {
            if (contacts[i].desiredVelocity > max)
            {
                max = contacts[i].desiredVelocity;
                index = i;
            }
        }

        if (index == numContacts) break;

        BodyProperties* bodyA = &componentManager->bodyProperties[contacts[index].bodyPair.a];
        BodyProperties* bodyB = &componentManager->bodyProperties[contacts[index].bodyPair.b];

        //do the resolution on the contact that came out top
        ApplyVelocityChange(velocityChange, rotationChange, &contacts[index]);

        for (size_t i = 0; i < numContacts; ++i)
        {
            for (size_t j = 0; j < 2; ++j)
            {
                //check for a match with each body
                for (size_t k = 0; k < 2; ++k)
                {
                    uint32_t a = (j == 0) ? contacts[i].bodyPair.a : contacts[i].bodyPair.b;
                    uint32_t b = (k == 0) ? contacts[index].bodyPair.a : contacts[index].bodyPair.b;
                    if (a == b)
                    {
                        deltaVel = velocityChange[k] +
                            rotationChange[k] * contacts[i].relativeContactPosition[j];

                        //the sign of the change is negative if we're dealing
                        //with the 2nd body
                        contacts[i].contactVelocity +=
                            glm::transpose(contacts[i].contactWorld) * deltaVel *
                            (j ? -1.0f : 1.0f);

                        CalculateDesiredDeltaVelocity(dt, &contacts[i]);
                    }
                }
            }
        }
        velocityIterationsUsed++;
    }

}

void Physics::ContactSolver::AdjustPositions(
    Contacts * contacts,
    size_t numContacts,
    float dt)
{
    size_t index;
    glm::vec3 linearChange[2], angularChange[2];
    float max;
    glm::vec3 deltaPosition;

    positionIterationsUsed = 0;

    while (positionIterationsUsed < positionIterations)
    {
        max = positionEpsilon;
        index = numContacts;
        for (size_t i = 0; i < numContacts; ++i)
        {
            if (contacts[i].penetration > max)
            {
                max = contacts[i].penetration;
                index = i;
            }
        }

        if (index == numContacts) break;

        BodyProperties* bodyA = &componentManager->bodyProperties[contacts[index].bodyPair.a];
        BodyProperties* bodyB = &componentManager->bodyProperties[contacts[index].bodyPair.b];

        //Resolve the penetration 
        //do the resolution on the contact that came out top
        ApplyPositionChange(linearChange, angularChange, &contacts[index], max);

        // Again this action may have changed the penetration of other
        // bodies, so we update contacts.
        for (size_t i = 0; i < numContacts; ++i)
        {
            for (size_t j = 0; j < 2; ++j)
            {
                //check for a match with each body
                for (size_t k = 0; k < 2; ++k)
                {
                    uint32_t a = (j == 0) ? contacts[i].bodyPair.a : contacts[i].bodyPair.b;
                    uint32_t b = (k == 0) ? contacts[index].bodyPair.a : contacts[index].bodyPair.b;
                    if (a == b)
                    {
                        deltaPosition = linearChange[k] +
                            angularChange[k] * contacts[i].relativeContactPosition[j];

                        //the sign of the change is negative if we're dealing
                        //with the 2nd body
                        contacts[i].penetration +=
                            glm::dot(deltaPosition, contacts[i].contactNormal) *
                            (j ? -1.0f : 1.0f);
                    }
                }
            }
        }
        positionIterationsUsed++;
    }

}