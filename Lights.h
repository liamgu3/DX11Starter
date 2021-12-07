#pragma once
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

#include <DirectXMath.h>

struct Light
{
	//variables for any type of light created
	int type;						//type of light
	DirectX::XMFLOAT3 direction;	//directional and spot lights need direction
	float range;					//point and spot lights have a max range for attenuation
	DirectX::XMFLOAT3 position;		//point and spot lights have a position in space
	float intensity;				//all lights need an intensity
	DirectX::XMFLOAT3 color;		//all lights need a color
	float spotFalloff;				//spot lights need a value to restrict their cone
	int castsShadows;				//does this cast shadows?
	DirectX::XMFLOAT2 padding;		//padding to hit 16-byte boundary
};