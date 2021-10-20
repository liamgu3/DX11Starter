#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <memory>
#include "Camera.h"
#include "Material.h"
#include "Lights.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh, Material* _material);
	~Entity();

	//setters
	Transform* GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	Material* GetMaterial();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera, DirectX::XMFLOAT3 ambient);

private:
	Transform transform;
	std::shared_ptr<Mesh> meshPtr;
	Material* material;
};

