#include "ShaderInclude.hlsli"

Texture2D SurfaceTexture	: register(t0);	//"t" registers for textures
Texture2D SurfaceRoughness	: register(t1);
Texture2D NormalMap			: register(t2);
SamplerState BasicSampler	: register(s0);	//"s" registers for samplers

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPos;
	float3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;
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

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
	return att * att;
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
	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	tangent = normalize(tangent - normal * dot(tangent, normal));	//Gram-Schmidt orthonormalize process
	float3 bi_tangent = cross(tangent, normal);
	float3x3 TBN = float3x3(tangent, bi_tangent, normal);

	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;

	input.normal = mul(unpackedNormal, TBN);

	//calculating diffuse lighting
	float3 diffuse1 = Diffuse(input.normal, directionalLight1.direction, directionalLight1.color) * directionalLight1.intensity;	//directionalLight1	
	float3 diffuse2 = Diffuse(input.normal, directionalLight2.direction, directionalLight2.color) * directionalLight2.intensity;	//directionalLight2
	float3 diffuse3 = Diffuse(input.normal, directionalLight3.direction, directionalLight3.color) * directionalLight3.intensity;	//directionalLight3 
	float3 diffuse4 = Diffuse(input.normal, normalize(input.worldPosition - pointLight1.position), pointLight1.color) * Attenuate(pointLight1, input.worldPosition) * pointLight1.intensity;	//pointLight1
	float3 diffuse5 = Diffuse(input.normal, normalize(input.worldPosition - pointLight2.position), pointLight2.color) * Attenuate(pointLight2, input.worldPosition) * pointLight2.intensity;	//pointLight2

	float pixelRoughness = 1.0f - (SurfaceRoughness.Sample(BasicSampler, input.uv).r);

	//calculating specular lighting
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 V = normalize(cameraPos - input.worldPosition);
	float specular1 = 0.0f;
	float specular2 = 0.0f;
	float specular3 = 0.0f;
	float specular4 = 0.0f;
	float specular5 = 0.0f;
	if (specExponent > 0.0f)	//if specExponent is 0 then we make spec =  0, otherwise it would become 1.0 (bright white)
	{
		specular1 = Specular(input.normal, directionalLight1.direction, V, specExponent) * pixelRoughness * any(diffuse1);	//directionalLight1	
		specular2 = Specular(input.normal, directionalLight2.direction, V, specExponent) * pixelRoughness * any(diffuse2);	//directionalLight2
		specular3 = Specular(input.normal, directionalLight3.direction, V, specExponent) * pixelRoughness * any(diffuse3);	//directionalLight3
		specular4 = Specular(input.normal, normalize(input.worldPosition - pointLight1.position), V, specExponent) * Attenuate(pointLight1, input.worldPosition) * pixelRoughness * any(diffuse4);	//pointLight1
		specular5 = Specular(input.normal, normalize(input.worldPosition - pointLight2.position), V, specExponent) * Attenuate(pointLight2, input.worldPosition) * pixelRoughness * any(diffuse5);	//pointLight2
	}

	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;

	float3 finalPixelColor = specular1 + specular2 + specular3 + specular4 + specular5 + diffuse1 + diffuse2 + diffuse3 + diffuse4 + diffuse5 + (ambient * colorTint) + (surfaceColor * colorTint);

	return float4(finalPixelColor, 1);
	//return float4(directionalLight1.color, 1);		//temporary test
}

