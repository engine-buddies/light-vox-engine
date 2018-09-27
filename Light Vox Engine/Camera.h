#pragma once
#include <DirectXMath.h>

/// <summary>
/// Simple wrapper for a 'camera'
/// </summary>
class Camera
{
private:
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 forward; 
    DirectX::XMFLOAT3 up; 

	float fov;      //field of view
	float nearZ;    //near plane (in Z)
    float farZ;     //far plane (in Z)

public:
	Camera();
	~Camera();

    /// <summary>
    /// Set where the camera is and what it's looking at
    /// </summary>
    /// <param name="position">The position of the camera</param>
    /// <param name="forward">Where the camera is pointing to</param>
    /// <param name="up">What 'up' is in this world</param>
	void SetTransform(DirectX::XMVECTOR position, 
        DirectX::XMVECTOR forward, 
        DirectX::XMVECTOR up);

    /// <summary>
	/// Calculates the view projection matrix
    /// </summary>
    /// <param name="view">The location to save the view matrix</param>
    /// <param name="proj">The location to save the projection matrix</param>
    /// <param name="screenWidth">The width of the screen</param>
    /// <param name="screenHeight">The height of the screen</param>
	void GetViewProjMatrix(DirectX::XMFLOAT4X4 *view, 
        DirectX::XMFLOAT4X4 *proj, 
        float screenWidth, 
        float screenHeight);
};

