#include "Entity.h"
#include "BufferStructs.h"
#include "Mesh.h"
using namespace DirectX;

Entity::Entity(std::shared_ptr<Mesh> mesh, Material* _material)
{
	meshPtr = mesh;
	transform = Transform();
	material = _material;
}

Entity::~Entity()
{
}

Transform* Entity::GetTransform()
{
	return &transform;
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return meshPtr;
}

Material* Entity::GetMaterial()
{
	return material;
}



void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera)
{
	//Step 2 - Put data into buffer struct
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();	//simplifies the next few lines
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());

	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	ps->SetFloat4("colorTint", material->GetColor());


	//step 3 - map, memcpy, unmap constant buffer
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	//activating vertex and pixelShader
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	//step 4+5 - set buffers and render with currently bound resources
	meshPtr->Draw();
}
