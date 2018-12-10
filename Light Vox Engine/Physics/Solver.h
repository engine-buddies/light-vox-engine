/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include "../stdafx.h"
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
        void Update(float dt);

    private:
        //SINGLE THREADED 

        //<summary>
        //Solves Collision (aabb to aabb) 
        //</summary>
        void Collide();
        // /<summary>
        //Semi Implicit euler intergration for position
        //<//summary>
        //<param name="dt"></param>
        void Integrate(float dt);
        // /<summary>
        //Calc. total force
        //<//summary>
        void AccumlateForces();
        // /<summary>
        //Calc. total torque
        //<//summary>
        void AccumlateTorque();
        // /<summary>
        //Calc model to world matrix
        //<//summary>
        void ModelToWorld();
        // /<summary>
        //Calcs the transformation matrix based on the given offset
        //and transform the bounding box vertices from local to world space
        //<//summary>
        void SetColliderData();

        void ResolveContacts(float dt);

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

        PhysicsArguments* a_argument = nullptr;
        PhysicsArguments* b_argument = nullptr;
    };
};