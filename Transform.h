#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	//constructor/destructor
	Transform();
	~Transform();

	//setters
	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);
	void SetScale(float x, float y, float z);

	//getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	//transformers
	void MoveAbsolute(float x, float y, float z);
	void Rotate(float pitch, float yaw, float roll);
	void Scale(float x, float y, float z);
	
private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 pitchYawRoll;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;
	bool matricesDirty;

	void UpdateMatrices();
};

