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
        glm::quat rot = { 1.0f, .0f, .0f, .0f };
        glm::vec3 pos = { .0f, .0f, .0f };
    };

    struct BoxCollider
    {
        glm::vec3 size = { .0f, .0f, .0f };
    };

    struct BodyProperties
    {
        glm::vec3 velocity = { .0f, .0f, .0f };
        glm::vec3 force = { .0f, .0f, .0f };
        glm::vec3 acceleration = { .0f, .0f, .0f };
        float mass = 1.0f;
    };
};
