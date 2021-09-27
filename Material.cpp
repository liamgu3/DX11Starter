#include "Material.h"

Material::Material(DirectX::XMFLOAT4 _colorTint, Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader, Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader)
{
	colorTint = _colorTint;
	pixelShader = _pixelShader;
	vertexShader = _vertexShader;
}

Material::~Material()
{
}

DirectX::XMFLOAT4 Material::GetColor()
{
	return colorTint;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

void Material::SetColor(DirectX::XMFLOAT4 _colorTint)
{
	colorTint = _colorTint;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader)
{
	pixelShader = _pixelShader;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader)
{
	vertexShader = _vertexShader;
}
