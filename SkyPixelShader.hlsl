TextureCube CubeMap			: register(t0);	//"t" registers for textures
SamplerState BasicSampler	: register(s0);	//"s" registers for samplers

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 sampleDir	: DIRECTION;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 cubeSample = CubeMap.Sample(BasicSampler, input.sampleDir) * 2 - 1;
	return cubeSample;
}