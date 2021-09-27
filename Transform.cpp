#include "Transform.h"
using namespace DirectX;

Transform::Transform()
{	
	//initializing scalars and world matrix
	position = XMFLOAT3(0, 0, 0);
	pitchYawRoll = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);

	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());

	matricesDirty = false;
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	matricesDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	pitchYawRoll = XMFLOAT3(pitch, yaw, roll);
	matricesDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	matricesDirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateMatrices();
	return worldInverseTransposeMatrix;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	XMFLOAT3 right;
	XMVECTOR localRight = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));
	XMStoreFloat3(&right, localRight);

	return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	XMFLOAT3 up;
	XMVECTOR localUp = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));
	XMStoreFloat3(&up, localUp);

	return up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 forward;
	XMVECTOR localForward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));
	XMStoreFloat3(&forward, localForward);

	return forward;
}

//translate object in world space
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	matricesDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	pitchYawRoll.x += pitch;
	pitchYawRoll.y += yaw;
	pitchYawRoll.z += roll;
	matricesDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	matricesDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR rotatedVector = XMVector3Rotate(XMVectorSet(x, y, z, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));
	XMStoreFloat3(&position, XMLoadFloat3(&position) + rotatedVector);
	matricesDirty = true;
}

void Transform::UpdateMatrices()
{
	if (!matricesDirty)	//if matrices haven't changed, return
		return;

	XMMATRIX transMat = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
	XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX worldMat = scaleMat * rotMat * transMat;	//SRT order important
	
	XMStoreFloat4x4(&worldMatrix, worldMat);
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

	matricesDirty = false;
}
