#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "DDSTextureLoader.h"
#include "Camera.h"

class Sky
{
public:
	Sky(std::shared_ptr<Mesh> mesh, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState, 
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _skySRV, 
		std::shared_ptr<SimplePixelShader> _pixelShader,
		std::shared_ptr<SimpleVertexShader> _vertexShader);

	~Sky();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera);

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
};

