#include "ComponentManager.h"

ComponentManager* ComponentManager::Instance = nullptr;

void ComponentManager::Init()
{
    //TO DO:: Intialize components
}

ComponentManager* ComponentManager::GetInstance()
{
    if (Instance == nullptr)
        Instance = new ComponentManager();

    return Instance;
}

void ComponentManager::Release()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

