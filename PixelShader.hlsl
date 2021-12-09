#include "ShaderInclude.hlsli"

static const float F0_NON_METAL = 0.04f;
static const float MIN_ROUGHNESS = 0.0000001f; // Minimum roughness for when spec distribution function denominator goes to zero
static const float PI = 3.14159265359f; // Handy to have this as a constant

Texture2D SurfaceTexture	: register(t0);	//"t" registers for textures
Texture2D SurfaceRoughness	: register(t1);
Texture2D NormalMap			: register(t2);
Texture2D MetalnessMap		: register(t3);
Texture2D ShadowMap			: register(t4);

SamplerState BasicSampler				: register(s0);	//"s" registers for samplers
SamplerComparisonState ShadowSampler	: register(s1);

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPos;
	float3 ambient;
	//Light directionalLight1;
	//Light directionalLight2;
	Light directionalLight3;
	//Light pointLight1;
	//Light pointLight2;
}

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount based on energy conservation
//
// diffuse - Diffuse amount
// specular - Specular color (including light color)
// metalness - surface metalness amount
//
// Metals should have an albedo of (0,0,0)...mostly
// See slide 65: http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
float3 DiffuseEnergyConserve(float3 diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

// GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1; MIN_ROUGHNESS helps here

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n (full specular color)
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2, roughness remapped to (r+1)/2
//
// n - Normal
// v - View vector
//
// G(l,v)
float GeometricShadowing(float3 n, float3 v, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}

// Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
//
// D() - Spec Dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor)
{
	// Other vectors
	float3 h = normalize(v + l);

	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, roughness) * GeometricShadowing(n, l, roughness);

	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
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

	float pixelRoughness = 1.0f - (SurfaceRoughness.Sample(BasicSampler, input.uv).r);

	float pixelMetalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	float3 surfaceColor = pow(SurfaceTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);

	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, pixelMetalness);

	//shadow mapping
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;

	//calculate pixel's depth from light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	//sample shadow map using comparison sampler, comparing depth from light and value in shadow map
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	//might need to go opposite direction
	//float diffuse1 = DiffusePBR(normalize(input.normal), normalize(-directionalLight1.direction));		//directionalLight1	
	//float diffuse2 = DiffusePBR(normalize(input.normal), normalize(-directionalLight2.direction));		//directionalLight2
	float diffuse3 = DiffusePBR(normalize(input.normal), normalize(-directionalLight3.direction));		//directionalLight3 
	//float diffuse4 = DiffusePBR(normalize(input.normal), normalize(input.worldPosition - pointLight1.position));// *Attenuate(pointLight1, input.worldPosition);	//pointLight1
	//float diffuse5 = DiffusePBR(normalize(input.normal), normalize(input.worldPosition - pointLight2.position));// *Attenuate(pointLight2, input.worldPosition);	//pointLight2

	//OLD DIFFUSE CALCULATIONS
	////calculating diffuse lighting
	//float3 diffuse1 = Diffuse(input.normal, directionalLight1.direction, directionalLight1.color) * directionalLight1.intensity;	//directionalLight1	
	//float3 diffuse2 = Diffuse(input.normal, directionalLight2.direction, directionalLight2.color) * directionalLight2.intensity;	//directionalLight2
	//float3 diffuse3 = Diffuse(input.normal, directionalLight3.direction, directionalLight3.color) * directionalLight3.intensity;	//directionalLight3 
	//float3 diffuse4 = Diffuse(input.normal, normalize(input.worldPosition - pointLight1.position), pointLight1.color) * Attenuate(pointLight1, input.worldPosition) * pointLight1.intensity;	//pointLight1
	//float3 diffuse5 = Diffuse(input.normal, normalize(input.worldPosition - pointLight2.position), pointLight2.color) * Attenuate(pointLight2, input.worldPosition) * pointLight2.intensity;	//pointLight2

	float3 V = normalize(cameraPos - input.worldPosition);

	//float3 specular1 = MicrofacetBRDF(input.normal, -directionalLight1.direction, V, pixelRoughness, specularColor);		//directionalLight1
	//float3 specular2 = MicrofacetBRDF(input.normal, -directionalLight2.direction, V, pixelRoughness, specularColor);		//directionalLight2
	float3 specular3 = MicrofacetBRDF(input.normal, -directionalLight3.direction, V, pixelRoughness, specularColor);		//directionalLight3
	//float3 specular4 = MicrofacetBRDF(input.normal, normalize(input.worldPosition - pointLight1.position), V, pixelRoughness, specularColor);// *Attenuate(pointLight1, input.worldPosition);	//pointLight1
	//float3 specular5 = MicrofacetBRDF(input.normal, normalize(input.worldPosition - pointLight2.position), V, pixelRoughness, specularColor);// *Attenuate(pointLight2, input.worldPosition);	//pointLight2

	//OLD SPECULAR CALCULATIONS
	////calculating specular lighting
	//float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	//float3 V = normalize(cameraPos - input.worldPosition);
	//float specular1 = 0.0f;
	//float specular2 = 0.0f;
	//float specular3 = 0.0f;
	//float specular4 = 0.0f;
	//float specular5 = 0.0f;
	//if (specExponent > 0.0f)	//if specExponent is 0 then we make spec =  0, otherwise it would become 1.0 (bright white)
	//{
	//	specular1 = Specular(input.normal, directionalLight1.direction, V, specExponent) * pixelRoughness * any(diffuse1);	//directionalLight1	
	//	specular2 = Specular(input.normal, directionalLight2.direction, V, specExponent) * pixelRoughness * any(diffuse2);	//directionalLight2
	//	specular3 = Specular(input.normal, directionalLight3.direction, V, specExponent) * pixelRoughness * any(diffuse3);	//directionalLight3
	//	specular4 = Specular(input.normal, normalize(input.worldPosition - pointLight1.position), V, specExponent) * Attenuate(pointLight1, input.worldPosition) * pixelRoughness * any(diffuse4);	//pointLight1
	//	specular5 = Specular(input.normal, normalize(input.worldPosition - pointLight2.position), V, specExponent) * Attenuate(pointLight2, input.worldPosition) * pixelRoughness * any(diffuse5);	//pointLight2
	//}

	//calculate diffuse with energy conservation
	//float3 balancedDiff1 = DiffuseEnergyConserve(diffuse1, specular1, pixelMetalness);		//directionalLight1
	//float3 balancedDiff2 = DiffuseEnergyConserve(diffuse2, specular2, pixelMetalness);		//directionalLight2
	float3 balancedDiff3 = DiffuseEnergyConserve(diffuse3, specular3, pixelMetalness);		//directionalLight3
	//float3 balancedDiff4 = DiffuseEnergyConserve(diffuse4, specular4, pixelMetalness);		//pointLight1
	//float3 balancedDiff5 = DiffuseEnergyConserve(diffuse5, specular5, pixelMetalness);		//pointLight2

	//combine the final diffuse and specular values for this light
	//float3 total1 = (balancedDiff1 * surfaceColor + specular1) * directionalLight1.intensity * directionalLight1.color;
	//float3 total2 = (balancedDiff2 * surfaceColor + specular2) * directionalLight2.intensity * directionalLight2.color;
	float3 total3 = (balancedDiff3 * surfaceColor + specular3) * directionalLight3.intensity * directionalLight3.color;
	//float3 total4 = (balancedDiff4 * surfaceColor + specular4) * pointLight1.intensity * pointLight1.color;
	//float3 total5 = (balancedDiff5 * surfaceColor + specular5) * pointLight2.intensity * pointLight2.color;

	//OLD FINAL COLOR CALCULATION
	//float3 finalPixelColor = specular1 + specular2 + specular3 + specular4 + specular5 + diffuse1 + diffuse2 + diffuse3 + diffuse4 + diffuse5 + (ambient * colorTint) + (surfaceColor * colorTint);
	//float3 finalPixelColor = total1 + total2 + total3 + total4 + total5; // +(ambient * colorTint);	//might not need ambient
	
	float3 finalPixelColor = total3 * (directionalLight3.castsShadows ? shadowAmount : 1.0f);	//only overhead light

	return float4(pow(finalPixelColor, 1.0f / 2.2), 1);
	//return float4(directionalLight1.color, 1);		//temporary test
}

