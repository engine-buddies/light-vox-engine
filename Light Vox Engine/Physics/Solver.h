/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include "../ECS/Entity.h"
#include "../ECS/ComponentManager.h"
#include "../Graphics/DebugRenderer.h"
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
        /// <summary>
        /// Calc model to world matrix
        /// </summary>
        void ModelToWorld();

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
        /// Calcs the transformation matrix based on the given offset
        /// and transform the bounding box vertices from local to world space
        /// </summary>
        void SetColliderData();


        glm::vec3 gravity;
        ECS::ComponentManager* componentManager;
        Physics::Rigidbody* rigidBody;
    };
};