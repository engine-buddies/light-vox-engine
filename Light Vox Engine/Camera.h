#pragma once
#include "stdafx.h"

class Camera
{
private:
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR forward;
	DirectX::XMVECTOR up;

	float fov;
	float nearZ;
	float farZ;

	float aspectRatio;

public:
	Camera();
	~Camera();

	void UpdateScreenDimensions(float width, float height);
	void SetTransform(DirectX::XMVECTOR position, DirectX::XMVECTOR forward, DirectX::XMVECTOR up);
	void GetViewProjMatrix(DirectX::XMFLOAT4X4 *viewProj, float screenWidth, float screenHeight);
	void GetViewProjMatrix(DirectX::XMFLOAT4X4 *viewProj);
};

