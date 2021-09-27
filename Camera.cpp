#include "Camera.h"
using namespace DirectX;

Camera::Camera(float x, float y, float z, float aspectRatio, float _moveSpeed = 2, float _lookSpeed = 2, float _fov = XM_PIDIV4)
{
	transform.SetPosition(x, y, z);
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);

	movementSpeed = _moveSpeed;
	mouseLookSpeed = _lookSpeed;
	fov = _fov;
}

Camera::~Camera()
{
}

void Camera::Update(float dt)
{
	//Movement
	Input& input = Input::GetInstance();
	float speed = movementSpeed * dt;

	if (input.KeyDown('W')) { transform.MoveRelative(0, 0, speed); }
	if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed); }
	if (input.KeyDown('A')) { transform.MoveRelative(speed, 0, 0); }
	if (input.KeyDown('D')) { transform.MoveRelative(-speed, 0, 0); }
	if (input.KeyDown(' ')) { transform.MoveAbsolute(0, -speed, 0); }
	if (input.KeyDown('X')) { transform.MoveAbsolute(0, speed, 0); }

	//Mouse Look
	if (input.MouseLeftDown())
	{
		float xDiff = dt * mouseLookSpeed * input.GetMouseXDelta();
		float yDiff = dt * mouseLookSpeed * input.GetMouseYDelta();

		transform.Rotate(-yDiff, -xDiff, 0);

		//constraining pitch so camera doesn't flip upside down
		if (transform.GetPitchYawRoll().x > XM_PIDIV2)
			transform.SetRotation(XM_PIDIV2, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
		else if (transform.GetPitchYawRoll().x < -XM_PIDIV2)
			transform.SetRotation(-XM_PIDIV2, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
	}
	
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform.GetPosition();	//gets transform's positon
	XMFLOAT3 fwd = transform.GetForward();	//gets transform's forward vector
	XMMATRIX V = XMMatrixLookToLH(
		XMLoadFloat3(&pos),
		XMLoadFloat3(&fwd),
		XMVectorSet(0, 1, 0, 0)
	);
	XMStoreFloat4x4(&viewMatrix, V);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(fov, aspectRatio, 0.01f, 100.0f);	//last two parameters are near and far plane
	XMStoreFloat4x4(&projectionMatrix, P);
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projectionMatrix;
}

Transform* Camera::GetTransform()
{
	return &transform;
}

float Camera::GetFOV()
{
	return fov;
}

void Camera::SetFOV(float _fov)
{
	fov = _fov;
	UpdateProjectionMatrix(aspectRatio);
}

