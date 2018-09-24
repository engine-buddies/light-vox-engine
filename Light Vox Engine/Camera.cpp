#include "Camera.h"

Camera::Camera()
{
	fov = 60;
	nearZ = 0.01f;
	farZ = 100.f;

	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	DirectX::XMFLOAT3 forward = DirectX::XMFLOAT3(0.f, 0.f, 1.f);
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
	SetTransform(
		DirectX::XMLoadFloat3(&pos),
		DirectX::XMLoadFloat3(&forward),
		DirectX::XMLoadFloat3(&up)
	);
}

Camera::~Camera() { }

void Camera::UpdateScreenDimensions(float width, float height)
{
	aspectRatio = width / height;
}

void Camera::SetTransform(DirectX::XMVECTOR position, DirectX::XMVECTOR forward, DirectX::XMVECTOR up)
{
	this->position = position;
	this->forward = forward;
	this->up = up;
}

void Camera::GetViewProjMatrix(DirectX::XMFLOAT4X4 * viewProj, float screenWidth, float screenHeight)
{
	//calculate vp mat and store in viewProj
	DirectX::XMStoreFloat4x4(viewProj, 
		DirectX::XMMatrixMultiplyTranspose(		//multiply and transpose two matrices
			DirectX::XMMatrixLookAtRH(position, forward, up),									//view
			DirectX::XMMatrixPerspectiveFovRH(fov, screenWidth / screenHeight, nearZ, farZ)		//projection
		)
	);
}

void Camera::GetViewProjMatrix(DirectX::XMFLOAT4X4 * viewProj)
{
	//calculate vp mat and store in viewProj
	DirectX::XMStoreFloat4x4(viewProj,
		DirectX::XMMatrixMultiplyTranspose(		//multiply and transpose two matrices
			DirectX::XMMatrixLookAtRH(position, forward, up),						//view
			DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearZ, farZ)		//projection
		)
	);
}
