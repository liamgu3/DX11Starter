cbuffer ExternalData : register(b0)
{
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
	float3 localPosition	: POSITION;     //XYZ position
	float2 uv				: TEXCOORD;		//UV
	float3 normal			: NORMAL;		//surface normal
	float3 tangent			: TANGENT;		//vector tangent to surface in u direction
};

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

VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	//copy of view matrix with translation zeroed
	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	matrix vp = mul(projection, viewNoTranslation);

	output.position = mul(vp, float4(input.localPosition, 1.0f));
	output.position.z = output.position.w;	//ensuring depth is 1.0

	output.sampleDir = input.localPosition;	//sampleDir is just pixels displacement from origin, so input.position works

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}