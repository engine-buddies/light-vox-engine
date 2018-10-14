#pragma once
#include <DirectXMath.h>
#include "stdafx.h"
#include "EntityComponents.h"

class ComponentManager
{
public:
    EntityComponents::Transform transform[LV_MAX_INSTANCE_COUNT];
    EntityComponents::BoxCollider boxCollider[LV_MAX_INSTANCE_COUNT];
    void Init();
    static ComponentManager* GetInstance();
    void Release();
private:
    static ComponentManager* Instance;
    static const uint32_t MAX_GAME_OBJECTS = LV_MAX_INSTANCE_COUNT;
};

