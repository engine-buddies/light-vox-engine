#pragma once

namespace Graphics
{
#define LV_DEBUG_MAX_CUBE_COUNT 100 

    struct CubeInstanceData
    {
        glm::float4x4 transform;
        glm::float3 color;
        float pad;
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

        void AddCube( glm::mat4x4 transform, glm::vec3 scale, glm::float3 color );
        void ClearCubes();

        CubeInstanceData* GetCubeInstanceDataPtr() { return cubeInstanceData; }
        size_t GetCubeInstanceDataCount() { return currCubeInstanceIndex + 1; }
    };
}
