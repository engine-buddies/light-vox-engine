#pragma once
#include <DirectXMath.h>
#include "stdafx.h"
#include "EntityComponents.h"

using namespace DirectX;

class ComponentManager
{
public:
    //SOA components
    EntityComponents::Transform transform[LV_MAX_INSTANCE_COUNT];
    EntityComponents::BoxCollider boxCollider[LV_MAX_INSTANCE_COUNT];
    EntityComponents::BodyProperties bodyProperties[LV_MAX_INSTANCE_COUNT];
    void Init();
    static ComponentManager* GetInstance();
    void Release();
private:
    static ComponentManager* Instance;
};

