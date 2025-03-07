#pragma once
#include "SimpleShader.h"

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <unordered_map>

class Material
{
public:
	Material(DirectX::XMFLOAT4 _colorTint, float roughness, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader);
	~Material();

	//Getters
	DirectX::XMFLOAT4 GetColor();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	float GetRoughness();

	//Setters
	void SetColor(DirectX::XMFLOAT4 _colorTint);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader);
	void SetRoughness(float _roughness);

	void PrepareMaterials();

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

private:
	DirectX::XMFLOAT4 colorTint;
	float roughness;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

