#include "EntityManager.h"

// Singleton requirement
EntityManager* EntityManager::Instance = nullptr;

EntityManager::EntityManager()
{
    // TODO: Create any necessary entity stuff here
}

EntityManager::~EntityManager()
{
    // TODO: Destroy entity objects here
}

EntityManager * EntityManager::GetInstance()
{
    if ( Instance == nullptr )
    {
        Instance = new EntityManager();
    }

    return Instance;
}

void EntityManager::Init()
{
    // TODO: Apply default properties of the entity manager here
}

void EntityManager::ReleaseInstance()
{
    if ( Instance != nullptr )
    {
        delete Instance;
        Instance = nullptr;
    }
}
