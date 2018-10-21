#include "ComponentManager.h"

ComponentManager* ComponentManager::Instance = nullptr;

void ComponentManager::Init()
{
    //TO DO:: Intialize components
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

ComponentManager::ComponentManager()
{
    for ( size_t i = 0; i < LV_MAX_INSTANCE_COUNT; ++i )
    {
        transform[ i ].transformMatrix = glm::mat4( 1.0 );
    }

}

ComponentManager::~ComponentManager()
{
}

