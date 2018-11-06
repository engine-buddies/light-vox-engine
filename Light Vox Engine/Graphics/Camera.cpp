#include "Camera.h"

using namespace DirectX;
using namespace Graphics;

Camera::Camera()
{
    //default values
    fov = 90.0f;
    nearZ = 0.01f;
    farZ = 100.f;

    position = XMFLOAT3( 0.f, 0.f, -2.f );
    forward = XMFLOAT3( 0.0f, 0.f, 1.f );
    up = XMFLOAT3( 0.f, 1.f, 0.f );
}

Camera::~Camera() {}

void Camera::SetTransform( XMVECTOR position, XMVECTOR forward, XMVECTOR up )
{
    XMStoreFloat3( &this->position, position );
    XMStoreFloat3( &this->forward, forward );
    XMStoreFloat3( &this->up, up );
}

void Camera::GetViewProjMatrix( DirectX::XMFLOAT4X4 * view,
    DirectX::XMFLOAT4X4 * proj,
    float screenWidth, float
    screenHeight )
{
    //calculate view
    XMStoreFloat4x4(
        view,
        XMMatrixTranspose( XMMatrixLookToLH(
            XMLoadFloat3( &position ),
            XMLoadFloat3( &forward ),
            XMLoadFloat3( &up ) ) )
    );

    float aspectRatio = screenWidth / screenHeight;
    float fovAngleY = 90.f * XM_PI / 180.0f;

    //calculate proj
    XMStoreFloat4x4(
        proj,
        XMMatrixTranspose(
            XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, nearZ, farZ )
        )
    );
}
