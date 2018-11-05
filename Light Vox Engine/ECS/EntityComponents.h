#pragma once
#include "../stdafx.h"
/// <summary>
/// Structures of the different components that an entity can have
/// </summary>
/// <author>Arturo</author>
/// <author>Ben Hoffman</author>

namespace EntityComponents
{
    struct Transform
    {
        glm::mat4 transformMatrix;
        glm::quat orientation = glm::quat();
        glm::vec3 rot = { .0f, .0f, .0f }; 
        glm::vec3 pos = { .0f, .0f, .0f };
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

        ////transformation matricies 
        //glm::mat4 posMatrix = glm::mat4(1.0f);
        //glm::mat4 rotMatrix = glm::mat4(1.0f);
        //glm::mat4 scaleMatrix = glm::mat4(1.0f);

        float angle = .0f;
    };
    
    struct BoxCollider
    {
        glm::mat4 offset = glm::mat4(1.0f);
        glm::mat4 transformMatrix;
        glm::vec3 contactPoint;
        glm::vec3 size = { .0f, .0f, .0f };
        uint32_t contactsLeft;
        uint16_t* tags;
        float penetration;
    };

    struct PlaneCollider
    {
        //plane normal
        glm::vec3 direction;
        float offset;
        uint16_t* tags;
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
