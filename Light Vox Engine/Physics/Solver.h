/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include "../stdafx.h"
#include <future>               // std::future, std::promise
#include "../ECS/Entity.h"
#include "../ECS/ComponentManager.h"
#include "../JobSystem/JobManager.h"
#include "ContactSolver.h"
#include "RigidBody.h"
#include "../Graphics/DebugRenderer.h"

namespace Physics
{
    class Solver
    {
    public:

        /// <summary>
        /// Intializes physics system
        /// -sets gravity 
        /// -intializies transform matrix
        /// </summary>
        Solver();
        ~Solver();

        /// <summary>
        /// updates physics calls by solving collision, solving forces,
        /// integrating position, and calc. model to world matrix
        /// </summary>
        /// <param name="dt"></param>
        void Update( float dt );

    private:

        /// <summary>
         /// Solves Collision (aabb to aabb) 
         /// </summary>
        static void Collide( Job* aJob, const void* aData );

        /// <summary>
        /// Semi Implicit euler intergration for position
        /// </summary>
        /// <param name="dt"></param>
        static void Integrate( Job* aJob, const void* aData );

        /// <summary>
        /// Calc. total force
        /// </summary>
        static void AccumlateForces( Job* aJob, const void* aData );

        /// <summary>
        /// Calc. total torque
        /// </summary>
        static void AccumlateTorque( Job* aJob, const void* aData );

        /// <summary>
        /// Calc model to world matrix
        /// </summary>
        static void ModelToWorld( Job* aJob, const void* aData );

        /// <summary>
        /// Calcs the transformation matrix based on the given offset
         /// and transform the bounding box vertices from local to world space
        /// </summary>
        static void SetColliderData( Job* aJob, const void* aData );

        /// <summary>
        /// Solves angular momentum, linear momentum, and interpenetration
        /// </summary>
        /// <param name="args"></param>
        /// <param name="index"></param>
        static void ResolveCollision( Job* aJob, const void* aData );

        static ECS::ComponentManager* componentManager;
        static Rigidbody* rigidbody;
        static ContactSolver* contactSolver;
        static Graphics::DebugRenderer* debugRenderer;

        struct PhysicsArguments
        {
            float DeltaTime;

            int StartElem;
            int EndElm;
        };

        std::vector<PhysicsArguments*> physicsJobsArgs;
    };
};