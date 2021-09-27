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



void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> constantBufferVS, Camera* camera)
{
	//Step 1 - Set Constant Buffer
	context->VSSetConstantBuffers(
		0,		//which slot (register) to bind the buffer to?
		1,		//How many are we activating? Can do multiple at once
		constantBufferVS.GetAddressOf());	// Array of buffers (or the address of one)

	//Step 2 - Put data into buffer struct
	VertexShaderExternalData vsData;
	vsData.colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.world = transform.GetWorldMatrix();
	vsData.viewMatrix = camera->GetView();
	vsData.projectionMatrix = camera->GetProjection();

	//step 3 - map, memcpy, unmap constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(constantBufferVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(constantBufferVS.Get(), 0);

	//activating vertex and pixelShader
	context->VSSetShader(material->GetVertexShader().Get(), 0, 0);
	context->PSSetShader(material->GetPixelShader().Get(), 0, 0);

	//step 4+5 - set buffers and render with currently bound resources
	meshPtr->Draw();

	//I think I can delete this
	////step 4 - set the correct vertex and index buffers
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, meshPtr->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	//context->IASetIndexBuffer(meshPtr->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
	//
	////step 5 tell Direct3D to render with currently bound resources
	//context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}
