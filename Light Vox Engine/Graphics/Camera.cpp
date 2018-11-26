#include "Camera.h"

using namespace Graphics;

Camera::Camera()
{
    //default values
    fov = 90.f * glm::pi<float>() / 180.0f;
    nearZ = 0.01f;
    farZ = 100.f;

    position = glm::vec3( 0.f, 0.f, -2.f );
    forward = glm::vec3( 0.0f, 0.f, 1.f );
    up = glm::vec3( 0.f, 1.f, 0.f );
}

Camera::~Camera() {}

void Camera::SetTransform( glm::vec3 position, glm::vec3 forward, glm::vec3 up )
{
    this->position = position;
    this->forward = forward;
    this->up = up;
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
