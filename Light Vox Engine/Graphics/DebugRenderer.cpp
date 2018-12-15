#include "../stdafx.h"
#include "DebugRenderer.h"

using namespace Graphics;

DebugRenderer* DebugRenderer::instance = nullptr;

DebugRenderer * DebugRenderer::GetInstance()
{
    if ( instance == nullptr )
        instance = new DebugRenderer();

    return instance;
}

void DebugRenderer::ReleaseInstance()
{
    if ( instance != nullptr )
        delete instance;
    instance = nullptr;
}

void DebugRenderer::AddCube( glm::mat4x4 transform, glm::vec3 scale, glm::float3 color )
{
#ifdef _DEBUG
    cubeInstanceData[ currCubeInstanceIndex ] = { };
    cubeInstanceData[ currCubeInstanceIndex ].transform = glm::transpose(glm::scale(transform, scale ));//  *transform;
    cubeInstanceData[ currCubeInstanceIndex ].color = color;
    ++currCubeInstanceIndex;
#endif
}

void DebugRenderer::ClearCubes()
{
    currCubeInstanceIndex = 0;
    memset( cubeInstanceData, 0, sizeof( CubeInstanceData ) * LV_DEBUG_MAX_CUBE_COUNT );
}

DebugRenderer::DebugRenderer()
{
    currCubeInstanceIndex = 0;
    memset( cubeInstanceData, 0, sizeof( CubeInstanceData ) * LV_DEBUG_MAX_CUBE_COUNT );
}

DebugRenderer::~DebugRenderer()
{

}
