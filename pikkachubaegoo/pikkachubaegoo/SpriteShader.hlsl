#define ATLASSPRITE_WIDTH 476
#define ATLASSPRITE_HEIGHT 885

cbuffer TransformConstant : register(b0)
{
    float4x4 transform;
}
cbuffer AtlasInfoConstant : register(b1)
{
    float4 atlasInfo;
}
Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);
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
    float2 uvmin = float2(atlasInfo.x / ATLASSPRITE_WIDTH, atlasInfo.y / ATLASSPRITE_HEIGHT);
    float2 uvsize = float2(atlasInfo.z / ATLASSPRITE_WIDTH, atlasInfo.w / ATLASSPRITE_HEIGHT);
    output.uv = uvmin + input.uv * uvsize;
    return output;
}

float4 mainPS(PSInput input) : SV_TARGET
{
    float3 color = SpriteTexture.Sample(SpriteSampler, input.uv).rgb;
    if (color.r + color.g + color.b < 0.2f)
    {
        discard;
    }
    return float4(color, 1);
}