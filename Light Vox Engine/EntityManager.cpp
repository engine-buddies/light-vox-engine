#include "EntityManager.h"

// Singleton requirement
EntityManager* EntityManager::Instance = nullptr;

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{
    
}

EntityManager * EntityManager::GetInstance()
{
    if ( Instance == nullptr )
    {
        Instance = new EntityManager();
    }

    return Instance;
}

void EntityManager::ReleaseInstance()
{
    if ( Instance != nullptr )
    {
        delete Instance;
        Instance = nullptr;
    }
}