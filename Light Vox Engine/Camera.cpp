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
	this->position = position;
	this->forward = forward;
	this->up = up;
}

void Camera::GetViewProjMatrix(XMFLOAT4X4 * viewProj, float screenWidth, float screenHeight)
{
	//calculate vp mat and store in viewProj
	XMStoreFloat4x4(viewProj, 
		XMMatrixMultiplyTranspose(		//multiply and transpose two matrices
			XMMatrixLookAtLH(position, forward, up),									//view
			XMMatrixPerspectiveFovLH(fov, screenWidth / screenHeight, nearZ, farZ)		//projection
		)
	);
}

