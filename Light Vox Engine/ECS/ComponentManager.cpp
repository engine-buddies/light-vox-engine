#include "../stdafx.h"
#include "ComponentManager.h"

using namespace ECS;
using namespace EntityComponents;

ComponentManager* ComponentManager::Instance = nullptr;

void ComponentManager::Init()
{
    //TO DO:: Intialize components
    // Create all components
}

ComponentManager* ComponentManager::GetInstance()
{
    if ( Instance == nullptr )
        Instance = new ComponentManager();

    return Instance;
}

void ComponentManager::ReleaseInstance()
{
    if ( Instance != nullptr )
    {
        delete Instance;
        Instance = nullptr;
    }
}

void ECS::ComponentManager::AddContactsFound()
{
    ++contactsFound;
}

void ECS::ComponentManager::ClearContactsFound()
{
    contactsFound = 0;
}

size_t ECS::ComponentManager::GetContactsFound()
{
    return contactsFound;
}

ComponentManager::ComponentManager()
{
    transformMatrix = new TransformMatrix[ LV_MAX_INSTANCE_COUNT ];
    transform = new Transform[ LV_MAX_INSTANCE_COUNT ];
    boxCollider = new BoxCollider[ LV_MAX_INSTANCE_COUNT ];
    bodyProperties = new BodyProperties[ LV_MAX_INSTANCE_COUNT ];
    contacts = new Contacts[ LV_MAX_INSTANCE_COUNT * 4];

    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        transformMatrix[ i ].transformMatrix = glm::mat4( 1.0 );
    }

}

ComponentManager::~ComponentManager()
{
    delete[] transformMatrix;
    delete[] transform;
    delete[] boxCollider;
    delete[] bodyProperties;
    delete[] contacts;
}

