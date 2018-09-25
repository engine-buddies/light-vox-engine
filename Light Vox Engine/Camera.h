#pragma once
#include <DirectXMath.h>

/* 
Simple wrapper for a 'camera'. Stores data for viewProj matrix and calculate it
*/
class Camera
{
private:
	//transform data (maybe they should be xmfloat3's?)

	DirectX::XMVECTOR position;
	DirectX::XMVECTOR forward;
	DirectX::XMVECTOR up;

	//data for view + proj
	
	float fov;
	float nearZ;
	float farZ;

public:
	Camera();
	~Camera();

	//set where the camera is and what it's looking at
	void SetTransform(DirectX::XMVECTOR position, DirectX::XMVECTOR forward, DirectX::XMVECTOR up);

	//calculates the viewProj mat
	void GetViewProjMatrix(DirectX::XMFLOAT4X4 *viewProj, float screenWidth, float screenHeight);
};

