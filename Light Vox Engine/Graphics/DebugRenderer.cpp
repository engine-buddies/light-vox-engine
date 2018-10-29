#include "DebugRenderer.h"

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

void DebugRenderer::AddCube( glm::mat4x4 transform, glm::vec3 scale, glm::vec3 color )
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
}

DebugRenderer::DebugRenderer()
{
    currCubeInstanceIndex = 0;
}

DebugRenderer::~DebugRenderer()
{

}
