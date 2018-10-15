#pragma once
#include "stdafx.h"
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
        glm::vec3 pos;
        glm::vec3 rot;
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
        float angle;
    };

    struct BoxCollider
    {
        glm::vec3 size;
    };

    struct BodyProperties
    {
        glm::vec3 velocity;
        glm::vec3 force;
        glm::vec3 acceleration;
        float mass = 1.0f;
    };
};
