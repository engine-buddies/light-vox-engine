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
        glm::mat4 transformMatrix;
    };

    struct Transform
    {
        glm::quat orientation = glm::quat();
        glm::vec3 rot = { .0f, .0f, .0f };
        glm::vec3 pos = { .0f, .0f, .0f };

        ////transformation matricies 
        //glm::mat4 posMatrix = glm::mat4(1.0f);
        //glm::mat4 rotMatrix = glm::mat4(1.0f);
        //glm::mat4 scaleMatrix = glm::mat4(1.0f);

        float angle = .0f;
    };

    struct BoxCollider
    {
        glm::vec3 vertices[8];
        glm::mat4 offset = glm::mat4(1.0f);
        glm::mat4 transformMatrix;
        glm::vec3 contactPoint;
        glm::vec3 maxVertex;
        glm::vec3 minVertex;
        //half size (the extent of the box along each axis)
        glm::vec3 size = { .50f, .50f, .50f };
        //holds collision tolerance 
        float tolerance;
        float friction;
        float restitution;
        uint16_t tag;
    };

    struct BodyPair
    {
        uint16_t a;
        uint16_t b;
    };

    struct Contacts
    {
        //number of contacts found so far
        BodyPair bodyPair;
        glm::vec3 contactPoint;
        glm::vec3 contactNormal;
        float penetration;
        float friction = 1.0f;
        float restitution = 1.0f;
        uint32_t contactsFound = 0;
        //Entity ID of colliding bodies
        uint16_t tag;
    };

    struct BodyProperties
    {
        glm::mat3 inertiaTensor = glm::mat3(1.0f);
        glm::vec3 velocity = { .0f, .0f, .0f };
        glm::vec3 force = { .0f, .0f, .0f };
        glm::vec3 acceleration = { .0f, .0f, .0f };
        //rotation
        glm::vec3 angularAcceleration = { .0f, .0f, .0f };
        glm::vec3 torque = { .0f, .0f, .0f };

        float linearDamping = 0.0f;
        float invMass = 1.0f;
        float mass = 1.0f;

        bool isAwake = true;
    };
};
