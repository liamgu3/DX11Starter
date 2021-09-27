#pragma once

#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>

class Camera
{
public:
	Camera(float x, float y, float z, float aspectRatio, float moveSpeed, float lookSpeed, float fov);
	~Camera();

	//Updates
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	//Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	Transform* GetTransform();
	float GetFOV();
	
	//Setters
	void SetFOV(float _fov);

private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	Transform transform;

	float movementSpeed;
	float mouseLookSpeed;
	float fov;
	float aspectRatio;
};

