#pragma once
#include "../stdafx.h"

namespace Graphics
{
    /// <summary>
    /// Simple wrapper for a 'camera'
    /// </summary>
    class Camera
    {
    private:
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;

        float fov;      //field of view
        float nearZ;    //near plane (in Z)
        float farZ;     //far plane (in Z)

    public:
        Camera();
        ~Camera();

        /// <summary>
        /// Set where the camera is and what it's looking at
        /// </summary>
        /// <param name="position">The position of the camera</param>
        /// <param name="forward">Where the camera is pointing to</param>
        /// <param name="up">What 'up' is in this world</param>
        void SetTransform( glm::vec3 position,
            glm::vec3 forward,
            glm::vec3 up
        );

        /// <summary>
        /// Calculates the view projection matrix
        /// </summary>
        /// <param name="view">The location to save the view matrix</param>
        /// <param name="proj">The location to save the projection matrix</param>
        /// <param name="screenWidth">The width of the screen</param>
        /// <param name="screenHeight">The height of the screen</param>
        void GetViewProjMatrix( glm::mat4x4_packed *view,
            glm::mat4x4_packed *proj,
            float screenWidth,
            float screenHeight
        );

        /// <summary>
        /// Gets camera position
        /// </summary>
        /// <returns>Float 3 representing camera position</returns>
        glm::vec3 GetPosition() const { return position; }
    };
}
