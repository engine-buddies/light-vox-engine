#pragma once

namespace Graphics
{
#define LV_POINT_LIGHT_COUNT 64

    struct LightingSceneConstantBuffer
    {
        glm::vec4_packed pointLightColors[ LV_POINT_LIGHT_COUNT ];
        glm::vec4_packed pointLightPositions[ LV_POINT_LIGHT_COUNT ];
    };

    class LightingManager
    {
    private:
        LightingManager();
        ~LightingManager();

        static LightingManager* instance;

        LightingSceneConstantBuffer lightingBuffer;

    public:
        static LightingManager* GetInstance();
        static void ReleaseInstance();

        void Update( float deltaTime );

        LightingSceneConstantBuffer* GetLightingBufferAddress() { return &lightingBuffer; }
    };

}
