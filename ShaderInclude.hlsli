#ifndef __GGP_SHADER_INCLUDES__		//each .hlsli needs a unique identifier
#define __GGP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2
#define MAX_SPECULAR_EXPONENT	256.0f

//all code pieces here

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;		//UV
	float3 normal			: NORMAL;		//surface normal
	float3 worldPosition	: POSITION;		//world position
	float3 tangent			: TANGENT;		//tangent to surface in u direction
};

struct Light
{
	//variables for any type of light created
	int type;						//type of light
	float3 direction;	//directional and spot lights need direction
	float range;					//point and spot lights have a max range for attenuation
	float3 position;		//point and spot lights have a position in space
	float intensity;				//all lights need an intensity
	float3 color;		//all lights need a color
	float spotFalloff;				//spot lights need a value to restrict their cone
	float3 padding;		//padding to hit 16-byte boundary
};

#endif