#include "Material.h"

Material::Material(DirectX::XMFLOAT4 _colorTint, float _roughness, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader)
{
	colorTint = _colorTint;
	roughness = _roughness;
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

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

float Material::GetRoughness()
{
	return roughness;
}

void Material::SetColor(DirectX::XMFLOAT4 _colorTint)
{
	colorTint = _colorTint;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader)
{
	pixelShader = _pixelShader;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader)
{
	vertexShader = _vertexShader;
}

void Material::SetRoughness(float _roughness)
{
	roughness = _roughness;
}
