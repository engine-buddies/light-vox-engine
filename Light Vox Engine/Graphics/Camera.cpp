#include "Camera.h"
#include "../Input/InputManager.h"

using namespace Graphics;


Camera::Camera()
{
    //default values
    fov = 60.f * glm::pi<float>() / 180.0f;
    nearZ = 0.01f;
    farZ = 100.f;

    position = glm::vec3( 0.f, 0.f, 0.f );
    forward = glm::vec3( DEFAULT_FORWARD );
    up = glm::vec3( DEFAULT_UP );
    right = glm::vec3( DEFAULT_RIGHT );

    pitchAngle = 0;
    yawAngle = 0;
    isLooking = false;

    inputManager = Input::InputManager::GetInstance();
    inputManager->BindAxis(
        Input::InputType::Look,
        this,
        &Graphics::Camera::OnLookAxis
    );

    inputManager->BindAxis( Input::InputType::StartLook, this, &Graphics::Camera::StartCameraLook );
    inputManager->BindAxis( Input::InputType::StopLook, this, &Graphics::Camera::StopCameraLook );

}

Camera::~Camera() {}

void Camera::SetTransform( glm::vec3 position, glm::vec3 forward, glm::vec3 up )
{
    this->position = position;
    this->forward = forward;
    this->up = up;
}

void Camera::MoveForward( float amount )
{
    this->position += this->forward * amount;
}

void Camera::MoveSideways( float amount )
{
    this->position += this->right * amount;
}

void Graphics::Camera::OnLookAxis()
{
    //only if right click is held down
    if ( !isLooking )
        return;

    Input::Point currPos = inputManager->GetCurrentMousePos();
    Input::Point prevPos = inputManager->GetPrevMousePos();

    pitchAngle += static_cast<float>( prevPos.y - currPos.y ) * SENSITIVITY;
    glm::clamp( pitchAngle, -MAX_PITCH, MAX_PITCH );
    yawAngle += static_cast<float>( currPos.x - prevPos.x ) * SENSITIVITY;

    //rotate along x and y
    glm::mat4x4 rotation = glm::eulerAngleYX( yawAngle, pitchAngle );
    forward = rotation * DEFAULT_FORWARD;
    up = rotation * DEFAULT_UP;
    right = glm::cross( forward, up );
}

void Graphics::Camera::StartCameraLook()
{
    isLooking = true;
}

void Graphics::Camera::StopCameraLook()
{
    isLooking = false;
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
