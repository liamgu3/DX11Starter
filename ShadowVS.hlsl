#include "ShaderInclude.hlsli"

cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float2 uv				: TEXCOORD;		//UV
	float3 normal			: NORMAL;		//surface normal
	float3 tangent			: TANGENT;		//vector tangent to surface in u direction
};

VertexToPixel_Shadow main( VertexShaderInput input)
{
	//setup output
	VertexToPixel_Shadow output;

	//calculate output position
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	return output;
}