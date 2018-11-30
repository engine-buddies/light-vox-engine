#pragma once
#include "../stdafx.h"

//forward declare input manager
namespace Input { class InputManager; }

namespace Graphics
{
    /// <summary>
    /// Simple wrapper for a 'camera'
    /// </summary>
    class Camera
    {
    private:
        const glm::vec4 DEFAULT_UP      = glm::vec4( 0.f, 1.f,  0.f, 0.f );
        const glm::vec4 DEFAULT_FORWARD = glm::vec4( 0.f, 0.f, -1.f, 0.f );
        const glm::vec4 DEFAULT_RIGHT   = glm::vec4( 1.f, 1.f,  0.f, 0.f );

        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;

        const float MAX_PITCH = glm::pi<float>() / 2.0f;
        const float SENSITIVITY = 0.01f;

        float pitchAngle;
        float yawAngle;

        float fov;      //field of view
        float nearZ;    //near plane (in Z)
        float farZ;     //far plane (in Z)

        Input::InputManager *inputManager;

        bool isLooking;

        /// <summary>
        /// Calculate direction based off of the yaw and pitch angle
        /// </summary>
        void CalculateDirection();

    public:
        Camera();
        ~Camera();

        /// <summary>
        /// Set where the camera is and what it's looking at
        /// </summary>
        /// <param name="position">The position of the camera</param>
        /// <param name="forward">Where the camera is pointing to</param>
        /// <param name="up">What 'up' is in this world</param>
        void SetTransform( glm::vec3 position, glm::vec3 forward, glm::vec3 up );

        /// <summary>
        /// Move forward by a certain amount
        /// </summary>
        /// <param name="amount">Amount to move forward by</param>
        void MoveForward( float amount );

        /// <summary>
        /// Move sideways by a certain amount
        /// </summary>
        /// <param name="amount">Amount to move sideways by</param>
        void MoveSideways( float amount );

        /// <summary>
        /// Rotate the camera long it's right
        /// </summary>
        /// <param name="angle">Angle to rotate by</param>
        void RotateAlongRight( float angle );

        /// <summary>
        /// Rotate along the global Y-axis
        /// </summary>
        /// <param name="angle">Angle to rotate by</param>
        void RotateAlongUp( float angle );

        /// <summary>
        /// Signaled by the input manager when the player uses
        /// the look axis
        /// </summary>
        void OnLookAxis();

        void StartCameraLook();

        void StopCameraLook();


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
