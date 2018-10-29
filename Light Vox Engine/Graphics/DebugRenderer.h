#pragma once
#include "../stdafx.h"

#define LV_DEBUG_MAX_CUBE_COUNT 100 

struct CubeInstanceData
{
    glm::mat4x4 transform;
    glm::vec3 color;
};

class DebugRenderer
{
private:
    DebugRenderer();
    ~DebugRenderer();

    static DebugRenderer *instance;

    CubeInstanceData cubeInstanceData[ LV_DEBUG_MAX_CUBE_COUNT ];
    size_t currCubeInstanceIndex;
public:
    static DebugRenderer* GetInstance();
    static void ReleaseInstance();

    void AddCube( glm::mat4x4 transform, glm::vec3 scale, glm::vec3 color );
    void ClearCubes();

    CubeInstanceData* GetCubeInstanceDataPtr() { return cubeInstanceData; }
    size_t GetCubeInstanceDataCount() { return currCubeInstanceIndex + 1; }
};
