#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <memory>
#include "Camera.h"
#include "Material.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh, Material* _material);
	~Entity();

	//setters
	Transform* GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	Material* GetMaterial();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> constantBufferVS, Camera* camera);

private:
	Transform transform;
	std::shared_ptr<Mesh> meshPtr;
	Material* material;
};

