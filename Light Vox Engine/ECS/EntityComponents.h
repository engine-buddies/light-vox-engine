#pragma once
#include "../stdafx.h"
/// <summary>
/// Structures of the different components that an entity can have
/// </summary>
/// <author>Arturo</author>
/// <author>Ben Hoffman</author>

namespace EntityComponents
{
    struct TransformMatrix
    {
        glm::packed_mat4 transformMatrix;
    };

    struct Transform
    {
        glm::quat orientation = glm::quat();
        glm::aligned_vec3 rot = { .0f, .0f, .0f };
        glm::aligned_vec3 pos = { .0f, .0f, .0f };

        ////transformation matricies 
        //glm::mat4 posMatrix = glm::mat4(1.0f);
        //glm::mat4 rotMatrix = glm::mat4(1.0f);

        float angle = .0f;
    };

    struct BoxCollider
    {
        glm::aligned_vec3 vertices[8];
        glm::aligned_mat4 offset = glm::aligned_mat4(1.0f);
        glm::aligned_mat4 transformMatrix;
        glm::aligned_vec3 contactPoint;
        glm::aligned_vec3 maxVertex;
        glm::aligned_vec3 minVertex;
        //half size (the extent of the box along each axis)
        glm::aligned_vec3 size = { .50f, .50f, .50f };
        //holds collision tolerance 
        float tolerance;
        float friction;
        float restitution;
        uint16_t tag;
    };

    struct BodyPair
    {
        uint32_t a;
        uint32_t b;
    };

    struct Contacts
    {
        glm::aligned_mat3 contactWorld;
        glm::aligned_vec3 relativeContactPosition[2];
        glm::aligned_vec3 contactPoint;
        glm::aligned_vec3 contactNormal;
        glm::aligned_vec3 contactVelocity;
        //entites colliding 
        BodyPair bodyPair;
        float desiredVelocity;
        float penetration;
        float friction = 1.0f;
        float restitution = 1.0f;
        //number of contacts found so far
        uint32_t contactsFound = 0;
    };

    struct BodyProperties
    {
        glm::aligned_mat3 inertiaTensor = glm::aligned_mat3(1.0f);
        glm::aligned_vec3 velocity = { .0f, .0f, .0f };
        glm::aligned_vec3 force = { .0f, .0f, .0f };
        glm::aligned_vec3 acceleration = { .0f, .0f, .0f };
        //rotation
        glm::aligned_vec3 angularAcceleration = { .0f, .0f, .0f };
        glm::aligned_vec3 torque = { .0f, .0f, .0f };

        float linearDamping = 0.0f;
        float invMass = 1.0f;
        float mass = 1.0f;

        bool isAwake = true;
    };
};