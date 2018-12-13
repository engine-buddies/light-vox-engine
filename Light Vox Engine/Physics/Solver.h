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
        void Collide( void* args, int index );
        /// <summary>
        /// Semi Implicit euler intergration for position
        /// </summary>
        /// <param name="dt"></param>
        void Integrate( void* args, int index );
        /// <summary>
        /// Calc. total force
        /// </summary>
        void AccumlateForces( void* args, int index );
        /// <summary>
        /// Calc. total torque
        /// </summary>
        void AccumlateTorque( void* args, int index );
        /// <summary>
        /// Calc model to world matrix
        /// </summary>
        void ModelToWorld( void* args, int index );
        /// <summary>
        /// Calcs the transformation matrix based on the given offset
         /// and transform the bounding box vertices from local to world space
        /// </summary>
        void SetColliderData( void* args, int index );
        /// <summary>
        /// Solves angular momentum, linear momentum, and interpenetration
        /// </summary>
        /// <param name="args"></param>
        /// <param name="index"></param>
        void ResolveCollision( void* args, int index );

        ECS::ComponentManager* componentManager;
        Jobs::JobManager* jobManager = nullptr;
        Rigidbody* rigidbody = nullptr;
        ContactSolver* contactSolver = nullptr;
        Graphics::DebugRenderer* debugRenderer = nullptr;

        struct PhysicsArguments
        {
            float DeltaTime;

            int StartElem;
            int EndElm;

            std::promise<void>* jobPromise;
        };

        std::vector< PhysicsArguments*> physicsJobsArgs;

        std::vector<std::promise<void>> promises;
        std::vector<std::future<void>> futures;
    };
};