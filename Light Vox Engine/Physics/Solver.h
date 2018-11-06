/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include "../ECS/Entity.h"
#include "../ECS/ComponentManager.h"
#include "RigidBody.h"

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

        /// <summary>
        /// Solves Collision (aabb to aabb) 
        /// </summary>
        void Collide();
        /// <summary>
        /// Semi Implicit euler intergration for position
        /// </summary>
        /// <param name="dt"></param>
        void Integrate(float dt);
        /// <summary>
        /// Calc. total force
        /// </summary>
        void AccumlateForces();
        /// <summary>
        /// Calc. total torque
        /// </summary>
        void AccumlateTorque();
        /// <summary>
        /// Calc model to world matrix
        /// </summary>
        void ModelToWorld();
        /// <summary>
        /// 
        /// </summary>
        void SetColliderData();


        glm::vec3 gravity;
        ECS::ComponentManager* componentManager;
        Physics::Rigidbody* rigidBody;
    };
};