#pragma once 
#include  "../stdafx.h"
#include "../ECS/Entity.h"
#include "../ECS/ComponentManager.h"

namespace Physics
{
    class Rigidbody
    {
    public:
        Rigidbody();
        ~Rigidbody();

       /// <summary>
       /// set a position for an entity
       /// </summary>
       /// <param name="pos"></param>
       /// <param name="e"></param>
       void Pos(glm::vec3 pos, size_t index);
       /// <summary>
       /// Set quaternion roation for entity
       /// </summary>
       /// <param name="rotationAxis"></param>
       /// <param name="angle"></param>
       /// <param name="e"></param>
        void RotateAxisAngle(
            glm::vec3 rotationAxis,
            float angle,
            size_t index);
        /// <summary>
        /// Set velocity
        /// </summary>
        /// <param name="vel"></param>
        /// <param name="index"></param>
        void Velocity(glm::vec3 vel, size_t index);
        /// <summary>
        /// Set acceleration
        /// </summary>
        /// <param name="accel"></param>
        /// <param name="index"></param>
        void Acceleration(glm::vec3 accel, size_t index);
        /// <summary>
        /// Set force
        /// </summary>
        /// <param name="force"></param>
        /// <param name="index"></param>
        void Force(glm::vec3 force, size_t index);
        /// <summary>
        /// Set mass
        /// </summary>
        /// <param name="mass"></param>
        /// <param name="index"></param>
        void Mass(float mass, size_t index);
        /// <summary>
        /// Set Scale
        /// </summary>
        /// <param name="scale"></param>
        /// <param name="index"></param>
        void Scale(glm::vec3 scale, size_t index);
        /// <summary>
        /// Set box collider size 
        /// </summary>
        /// <param name="size"></param>
        /// <param name="index"></param>
        void BoxColliderSize(glm::vec3 size, size_t index);

    private:
        ECS::ComponentManager* componentManager;

    };
};