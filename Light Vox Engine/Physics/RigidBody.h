#pragma once 
#include "../ECS/Entity.h"
#include "../ECS/ComponentManager.h"
#include "../ECS/EntityComponents.h"

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
        void Pos(glm::vec3& pos, const size_t& index);
        /// <summary>
        /// Set quaternion roation for entity
        /// </summary>
        /// <param name="rotationAxis"></param>
        /// <param name="angle"></param>
        /// <param name="e"></param>
        void RotateAxisAngle(
            glm::vec3& rotationAxis,
            float angle,
            const size_t& index);
        /// <summary>
        /// Set velocity
        /// </summary>
        /// <param name="vel"></param>
        /// <param name="index"></param>
        void Velocity(glm::vec3& vel, const size_t& index);
        /// <summary>
        /// Set acceleration
        /// </summary>
        /// <param name="accel"></param>
        /// <param name="index"></param>
        void Acceleration(glm::vec3& accel, const size_t& index);
        /// <summary>
        /// Set force
        /// </summary>
        /// <param name="force"></param>
        /// <param name="index"></param>
        void Force(glm::vec3& force, const size_t& index);
        /// <summary>
       /// Set mass
       /// </summary>
       /// <param name="mass"></param>
       /// <param name="index"></param>
        void Mass(float mass, const size_t& index);
        /// <summary>
        /// Set Scale
        /// </summary>
        /// <param name="scale"></param>
        /// <param name="index"></param>
        void Scale(glm::vec3& scale, const size_t& index);
        /// <summary>
        /// Set box collider size 
        /// </summary>
        /// <param name="size"></param>
        /// <param name="index"></param>
        void BoxColliderSize(glm::vec3& size, const size_t& index);
        /// <summary>
        /// Set 3x3 inertia tensor 
        /// </summary>
        /// <param name="intertiaTensor"></param>
        /// <param name="index"></param>
        void InertiaTensor(glm::mat3& inertiaTensor, const size_t& index);
        /// <summary>
        /// Sets the awake status of the object 
        /// </summary>
        /// <param name="awake"></param>
        /// <param name="index"></param>
        void SetAwake(const bool awake, const size_t& index);
        /// <summary>
        /// Adds a force on the object in world space
        /// </summary>
        /// <param name="force"></param>
        /// <param name="point"></param>
        void AddForceAtPoint(const glm::vec3 &force, const glm::vec3 &point, const size_t& index);
        /// <summary>
        /// Checks for bounding box collision b/w two entities
        /// AABB
        /// </summary>
        /// <param name="entityA"></param>
        /// <param name="entityB"></param>
        /// <returns></returns>
        bool IntersectBoxBox(const size_t& entityA, const size_t& entityB);
        /// <summary>
        /// Does a narrow phase collision detection between 2 boxes
        /// Finds the contact points, collision depths, and collision normals
        /// </summary>
        /// <param name="entityA"></param>
        /// <param name="entityB"></param>
        /// <returns></returns>
        int CollideBoxBox(const size_t& entityA, const size_t& entityB);
        /// <summary>
        /// Calcs. the vertices of the bounding box 
        /// </summary>
        /// <param name="index"></param>
        void CalcHalfSize(const size_t& index);

        void FillPointFaceBoxBox(const EntityComponents::BoxCollider& one,
            const EntityComponents::BoxCollider& two,
            const glm::vec3& toCenter,
            EntityComponents::Contacts* contactData,
            unsigned best,
            float penetration);

    private:
        ECS::ComponentManager* componentManager;

    };
};