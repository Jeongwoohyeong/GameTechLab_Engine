cbuffer HeightFogConstantBuffer : register(b7)
{
    float4 FogInscatteringColor;
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float3 padding;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float3 normal : NORMAL0;
    float4 color : COLOR; // Color to pass to the pixel shader
    float2 texCoord : TEXCOORD0;
};



Texture2D SceneDepthTexture : register(t0);
SamplerState DefaultSampler : register(s0);

cbuffer InvViewProjMatrixBuffer : register(b4)
{
    matrix InvViewProj;
    matrix InvWorld;
};

PS_INPUT mainVS(uint VertexID : SV_VertexID)
{
    PS_INPUT Out;
    float2 pos[3] =
    {
        float2(-1.0f, 3.0f),
        float2(3.0f, -1.0f),
        float2(-1.0f, -1.0f)
    };
    
    Out.position = float4(pos[VertexID], 0.1f, 1.0f);
    Out.texCoord = (pos[VertexID] + float2(1.0, -1.0)) * float2(0.5, -0.5);
    Out.normal = float3(0.0f, 0.0f, 1.0f); // Default normal
    Out.color = float4(1.0f, 0.0f, 0.0f, 0.2f); // Default color
    
    return Out;
}

float4 mainPS(PS_INPUT In) : SV_TARGET
{
    float depth = SceneDepthTexture.Sample(DefaultSampler, In.texCoord).r;
    return float4(depth, depth, depth, 1.0f);
}