// C++에서 상수 버퍼를 통해 전달될 데이터
cbuffer DecalInfo : register(b6)
{
    row_major matrix worldMVP;
    row_major matrix decalMVP;
    float Alpha;
    float3 Pad;
}

struct VS_INPUT
{
    float3 position : POSITION; // Input position from vertex buffer
    float3 normal : NORMAL0;
    float4 color : COLOR; // Input color from vertex buffer
    float2 texCoord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 decalNDC : TEXCOORD0;
    
    // 추후 decal이 추가 정보를 입력받을 경우를 대비한 주석
    // float3 normal : NORMAL0;
    // float4 color : COLOR; // Color to pass to the pixel shader
    // float2 texCoord : TEXCOORD1;
};

Texture2D g_DecalTexture : register(t0);
SamplerState g_Sample : register(s0);

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;;
    
    output.position = mul(float4(input.position, 1.0f), worldMVP);
    output.decalNDC = mul(float4(input.position, 1.0f), decalMVP);
    
    output.position.z =
    output.position.z - 0.001f >= 0.000001f ?
    output.position.z - 0.001f : output.position.z;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Perspective division: homogeneous coordinates를 NDC로 변환
    float3 decalNDC = input.decalNDC.xyz / input.decalNDC.w;

    // DirectX NDC 범위 체크: x,y ∈ [-1, 1], z ∈ [0, 1]
    if (
        decalNDC.x <= -1.0f ||
        decalNDC.x >= 1.0f ||
        decalNDC.y <= -1.0f ||
        decalNDC.y >= 1.0f ||
        decalNDC.z <= 0.0f ||
        decalNDC.z >= 1.0f
        )
    {
        discard;
    }
    
    // NDC [-1, 1]을 UV [0, 1]로 변환
    float2 uv;
    uv.x = decalNDC.x * 0.5f + 0.5f;
    uv.y = 1.0f - (decalNDC.y * 0.5f + 0.5f);
    
    float4 finalColor = g_DecalTexture.Sample(g_Sample, uv);
    clip(finalColor.a - 0.5f);    
    finalColor.a *= Alpha;
    
    
    return finalColor;
}
