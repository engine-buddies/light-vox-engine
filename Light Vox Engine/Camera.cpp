#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	//default values
	fov = 0.25f * XM_PI;
	nearZ = 0.01f;
	farZ = 100.f;

	XMFLOAT3 pos = XMFLOAT3(0.f, 0.f, -1.f);
	XMFLOAT3 forward = XMFLOAT3(0.f, 0.f, 1.f);
	XMFLOAT3 up = XMFLOAT3(0.f, 1.f, 0.f);
	SetTransform(
		XMLoadFloat3(&pos),
		XMLoadFloat3(&forward),
		XMLoadFloat3(&up)
	);
}

Camera::~Camera() { }

void Camera::SetTransform(XMVECTOR position, XMVECTOR forward, XMVECTOR up)
{
    XMStoreFloat3(&this->position, position);
    XMStoreFloat3(&this->forward, forward);
    XMStoreFloat3(&this->up, up);
}

void Camera::GetViewProjMatrix(
    DirectX::XMFLOAT4X4 * view, 
    DirectX::XMFLOAT4X4 * proj, 
    float screenWidth, 
    float screenHeight)
{
    //calculate view mat
    XMStoreFloat4x4(view,
        XMMatrixTranspose(
            XMMatrixLookAtLH(
                XMLoadFloat3(&position),
                XMLoadFloat3(&forward),
                XMLoadFloat3(&up)
            )
		)
	);

    //calculate the projection matrix
    XMStoreFloat4x4(proj,
        XMMatrixTranspose(
            XMMatrixPerspectiveFovLH(
                fov, 
                screenWidth / screenHeight, 
                nearZ, 
                farZ
            )		
        )
    );

}

