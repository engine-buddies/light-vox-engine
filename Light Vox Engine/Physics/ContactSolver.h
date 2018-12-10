/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include  "../stdafx.h"
#include "../ECS/EntityComponents.h"
#include "../ECS/ComponentManager.h"

using namespace EntityComponents;

namespace Physics
{
    class ContactSolver
    {
    public:
        ContactSolver(uint16_t iterations,
            float velEpsilon = (0.01f),
            float posEpsilon = (0.01f));
        ~ContactSolver();
        /// <summary>
        /// Resolved a set of contacts for both penetration and velocity
        /// </summary>
        /// <param name="contacts"> contactArray pointer to an array of contact objects</param>
        /// <param name="numContacts"> the number of contacts resolved</param>
        /// <param name="dt"> the number iterations through the resolution algorithm</param>
        void ResolveContacts(
            Contacts* contacts,
            size_t numContacts,
            float dt);
        /// <summary>
        /// Iterations the solver goes through for each contact
        /// </summary>
        /// <param name="iterations"></param>
        void SetIterations(size_t iterations);
    private:
        ECS::ComponentManager* componentManager;

        float velocityEpsilon;
        float positionEpsilon;

        size_t positionIterations = 0;
        size_t velocityIterations = 0;
        size_t velocityIterationsUsed = 0;
        size_t positionIterationsUsed = 0;

        struct ContactBodies
        {
            BodyProperties* bodyProps;
            Transform* transform;
        };

        /// <summary>
        /// Return true if the resolver has valid settings
        /// </summary>
        /// <returns></returns>
        bool isValid();
        /// <summary>
        /// Calcs velocity in local  space
        /// </summary>
        /// <param name="c"></param>
        /// <param name="bodyIndex"></param>
        /// <param name="bodyPair"></param>
        /// <param name="dt"></param>
        /// <returns></returns>
        glm::vec3 CalculateLocalVelocity(Contacts* c,
            uint32_t bodyIndex,
            uint16_t bodyPair,
            float dt);
        /// <summary>
        /// Solves microcollision
        /// </summary>
        /// <param name="dt"></param>
        /// <param name="c"></param>
        void CalculateDesiredDeltaVelocity(float dt, Contacts* c);
        /// <summary>
        /// Solves angular and linear momentum
        /// </summary>
        /// <param name="velocityChange"></param>
        /// <param name="rotationChange"></param>
        /// <param name="c"></param>
        void ApplyVelocityChange(
            glm::vec3 velocityChange[2],
            glm::vec3 rotationChange[2],
            Contacts* c);
        /// <summary>
        /// Solves interpenetration
        /// </summary>
        /// <param name="linearChange"></param>
        /// <param name="angularChange"></param>
        /// <param name="c"></param>
        /// <param name="penetration"></param>
        void ApplyPositionChange(
            glm::vec3 linearChange[2],
            glm::vec3 angularChange[2],
            Contacts* c,
            float penetration);
        /// <summary>
        /// Calc. internal data from state data
        /// </summary>
        /// <param name="dt"></param>
        void CalculateInternals(float dt, Contacts* contact);
        /// <summary>
        /// Sets up contacts ready for processing. 
        /// This makes sure their internal data is configured correctly
        /// and correct set of bodies is made alive.
        /// </summary>
        /// <param name="contacts"></param>
        /// <param name="numContacts"></param>
        /// <param name="dt"></param>
        void PrepareContacts(
            Contacts* contacts,
            size_t numContacts,
            float dt);
        /// <summary>
        /// Resolves velocity issues with the given array of constraints
        /// </summary>
        /// <param name="contacts"></param>
        /// <param name="numContacts"></param>
        /// <param name="duration"></param>
        void AdjustVelocities(
            Contacts* contacts,
            size_t numContacts,
            float dt);
        /// <summary>
        /// Resolves the potential issues with the given array of constraints
        /// </summary>
        /// <param name="contacts"></param>
        /// <param name="numContacts"></param>
        /// <param name="dt"></param>
        void AdjustPositions(
            Contacts* contacts,
            size_t numContacts,
            float dt);
    };
}