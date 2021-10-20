#include "ShaderInclude.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPos;
	float3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
}

float3 Diffuse(float3 normal, float3 dirToLight, float3 color)
{
	float3 normalizedDirection = normalize(-dirToLight);
	float3 NdotL = saturate(dot(normal, normalizedDirection));
	return NdotL * colorTint * color;
}

float3 Specular(float3 normal, float3 dirFromLight, float3 viewVec, float specExponent)
{
	float3 R = reflect(normalize(dirFromLight), normal);
	return pow(saturate(dot(R, viewVec)), specExponent) * colorTint;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	//calculating diffuse lighting
	float3 diffuse1 = Diffuse(input.normal, directionalLight1.direction, directionalLight1.color);
	float3 diffuse2 = Diffuse(input.normal, directionalLight2.direction, directionalLight2.color);
	float3 diffuse3 = Diffuse(input.normal, directionalLight3.direction, directionalLight3.color);

	//calculating specular lighting
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 V = normalize(cameraPos - input.worldPosition);
	float specular1 = 0.0f;
	float specular2 = 0.0f;
	float specular3 = 0.0f;
	if (specExponent > 0.0f)	//if specExponent is 0 then we make spec =  0, otherwise it would become 1.0 (bright white)
	{
		specular1 = Specular(input.normal, directionalLight1.direction, V, specExponent);
		specular2 = Specular(input.normal, directionalLight2.direction, V, specExponent);
		specular3 = Specular(input.normal, directionalLight3.direction, V, specExponent);
	}


	float3 finalPixelColor = specular1 + specular2 + specular3 + diffuse1 + diffuse2 + diffuse3 + (ambient * colorTint);

	return float4(finalPixelColor, 1);
	//return float4(directionalLight1.color, 1);		//temporary test
}

