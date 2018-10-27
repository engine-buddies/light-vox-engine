#pragma once
#include "../stdafx.h"
#include "EntityComponents.h"

namespace ECS {
    class ComponentManager
    {
    public:
        //SOA components
        EntityComponents::Transform transform[LV_MAX_INSTANCE_COUNT];
        EntityComponents::BoxCollider boxCollider[LV_MAX_INSTANCE_COUNT];
        EntityComponents::BodyProperties bodyProperties[LV_MAX_INSTANCE_COUNT];
        static ComponentManager* GetInstance();
        void Init();
        void ReleaseInstance();

        ComponentManager(ComponentManager const&) = delete;
        void operator=(ComponentManager const&) = delete;

    private:
        ComponentManager();
        ~ComponentManager();

        static ComponentManager* Instance;
    };

};