#include "../stdafx.h"
#include "ShaderDefinitions.h"
#include "LightingManager.h"

using namespace Graphics;

LightingManager* LightingManager::instance = nullptr;

LightingManager::LightingManager()
{
    lightingBuffer = { };
    for ( size_t i = 0; i < LV_POINT_LIGHT_COUNT; ++i )
    {
        glm::vec4_packed randomColor = glm::linearRand( glm::vec4_packed( 1.0f ), glm::vec4_packed( 0.0f ) ) * 50.0f;
        glm::vec4_packed randomPos = glm::linearRand(
            glm::vec4_packed( static_cast<float>( -LV_MAX_WORLD_SIZE ) ),
            glm::vec4_packed( static_cast<float>( LV_MAX_WORLD_SIZE ) )
        );

        lightingBuffer.pointLightColors[ i ] = randomColor;
        lightingBuffer.pointLightPositions[ i ] = randomPos;

        //DEBUG_PRINT( "Light (%f, %f, %f) at (%f, %f, %f)", randomColor.x, randomColor.y, randomColor.z, randomPos.x, randomPos.y, randomPos.z )
    }
}

LightingManager::~LightingManager()
{
}

LightingManager * LightingManager::GetInstance()
{
    if ( instance == nullptr )
        instance = new LightingManager();
    return instance;
}

void LightingManager::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}

void Graphics::LightingManager::Update( float deltaTime )
{
    for ( size_t i = 0; i < LV_POINT_LIGHT_COUNT; ++i )
    {
        glm::vec4_packed pos = lightingBuffer.pointLightPositions[ i ];

        pos.y += glm::sin( pos.z ) * ( pos.x + 1 ) * deltaTime;
        pos.x += glm::cos( pos.y ) * ( pos.z + 1 ) * deltaTime;
        pos.z -= glm::sin( pos.x ) * ( pos.y + 1 ) * deltaTime;

        glm::clamp( pos, glm::vec4_packed( -LV_MAX_WORLD_SIZE * 2.0f ), glm::vec4_packed( LV_MAX_WORLD_SIZE * 2.0f ) );

        lightingBuffer.pointLightPositions[ i ] = pos;
    }
}
