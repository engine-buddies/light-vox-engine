#pragma once
#include "../stdafx.h"
#include "EntityComponents.h"

namespace ECS {
    class ComponentManager
    {
    public:
        //SOA components
        EntityComponents::TransformMatrix transformMatrix[ LV_MAX_INSTANCE_COUNT ];
        EntityComponents::Transform transform[LV_MAX_INSTANCE_COUNT];
        EntityComponents::BoxCollider boxCollider[LV_MAX_INSTANCE_COUNT];
        EntityComponents::BodyProperties bodyProperties[LV_MAX_INSTANCE_COUNT];
        EntityComponents::Contacts contacts[LV_MAX_INSTANCE_COUNT * 4];
        static ComponentManager* GetInstance();
        void Init();
        void ReleaseInstance();

        ComponentManager(ComponentManager const&) = delete;
        void operator=(ComponentManager const&) = delete;

        int contactsFound = 0;

    private:
        ComponentManager();
        ~ComponentManager();

        static ComponentManager* Instance;
    };

};