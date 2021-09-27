#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

class Material
{
public:
	Material(DirectX::XMFLOAT4 _colorTint, Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader, Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader);
	~Material();

	//Getters
	DirectX::XMFLOAT4 GetColor();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();

	//Setters
	void SetColor(DirectX::XMFLOAT4 _colorTint);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader);
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader);

private:
	DirectX::XMFLOAT4 colorTint;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
};

