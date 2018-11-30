/// <summary>
/// By Arturo Kuang
/// </summary>
#pragma once
#include  "../stdafx.h"
#include "../ECS/EntityComponents.h"
#include "../ECS/ComponentManager.h"
#include "../ECS/EntityComponents.h"

using namespace EntityComponents;

namespace Physics
{
    class Contacts
    {
    public:
        Contacts();
        ~Contacts();
        
    private:
        struct Body
        {
            Transform* transform;
            BodyProperties* bodyProperties;
        };

        Body rigidBodies[2];
        float friction;
        float restitution;

        

    };
}