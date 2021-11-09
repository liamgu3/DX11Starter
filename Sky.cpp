#include "Sky.h"

Sky::Sky(std::shared_ptr<Mesh> mesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _skySRV, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader)
{
	skyMesh = mesh;
	samplerState = _samplerState;
	skySRV = _skySRV;
	pixelShader = _pixelShader;
	vertexShader = _vertexShader;

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC stencilDesc = {};
	stencilDesc.DepthEnable = true;
	stencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&stencilDesc, depthStencilState.GetAddressOf());
}

Sky::~Sky()
{
}

void Sky::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera)
{
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	//activating shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	pixelShader->SetSamplerState("BasicSampler", samplerState);
	pixelShader->SetShaderResourceView("CubeMap", skySRV);

	//putting data into buffer
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());

	//map, memcpy, unmap constant buffer
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();

	//draw the mesh
	skyMesh->Draw();

	//resetting render states
	//not entirely sure what should be reset here
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}
