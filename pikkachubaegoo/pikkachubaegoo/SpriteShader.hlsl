cbuffer Constant : register(b0)
{
	float4x4 transform;
}

struct VSInput
{
	float3 posModel : POSITION;
	float2 uv : TEXCOORD;
};
struct PSInput
{
	float4 posCS : SV_POSITION;
	float2 uv : TEXCOORD;
};

PSInput mainVS(VSInput input)
{
	PSInput output;
	output.posCS = mul(transform, float4(input.posModel, 1.0f));
	output.uv = input.uv;
	return output;
}

float4 mainPS(PSInput input) : SV_TARGET
{
	return float4(input.uv.x,input.uv.y, 1, 1);
}