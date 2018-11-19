#include "Camera.h"

using namespace Graphics;

Camera::Camera()
{
    //default values
    fov = 90.f * glm::pi<float>() / 180.0f;
    nearZ = 0.01f;
    farZ = 100.f;

    position = glm::vec3( 0.f, 0.f, 0.f );
    forward = glm::vec3( 0.0f, 0.f, -1.f );
    up = glm::vec3( 0.f, 1.f, 0.f );
    right = glm::vec3( 1.0f, 0.0f, 0.0f );
    pitchAngle = 0;
}

Camera::~Camera() {}

void Camera::SetTransform( glm::vec3 position, glm::vec3 forward, glm::vec3 up )
{
    this->position = position;
    this->forward = forward;
    this->up = up;
}

void Graphics::Camera::MoveForward( float amount )
{
    this->position += this->forward * amount;
}

void Graphics::Camera::MoveSideways( float amount )
{
    this->position += this->right * amount;
}

void Graphics::Camera::RotateAlongRight( float angle )
{
    if ( pitchAngle > -MAX_PITCH || pitchAngle < MAX_PITCH )
    {
        pitchAngle += angle;
        glm::quat rotation = glm::angleAxis( angle, right );
        this->forward = rotation * this->forward;
        this->up = rotation * this->up;
        this->right = rotation * this->right;
    }
}

void Graphics::Camera::RotateAlongUp( float angle )
{
    static glm::vec3 globalUp = glm::vec3( 0.0f, 1.0f, 0.0f );
    glm::quat rotation = glm::angleAxis( angle, globalUp );

    this->forward = rotation * this->forward;
    this->right = rotation * this->right;
}

void Camera::GetViewProjMatrix( glm::mat4x4_packed* view,
    glm::mat4x4_packed* proj,
    float screenWidth, float
    screenHeight )
{
    //calculate view
    *view = glm::transpose( glm::lookAtLH( position, position + forward, up ) );

    //calculate proj
    *proj = glm::transpose( glm::perspectiveFovLH( fov, screenWidth, screenHeight, nearZ, farZ ) );
}
