cbuffer ModelBuffer : register(b0)
{
    row_major float4x4 WorldMatrix;
}

cbuffer ViewProjBuffer : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
}
cbuffer FireBallConstants : register(b2)
{
    float4 FireBallWorldPosition;   // .xyzw (w=1.0f)
    float4 FireBallColor;           // .rgba
    float4 FireBallParams;          // .x = Intensity, .y = Radius, .z = InvRadius, .w = RadiusFallOff
}

cbuffer RealModelBuffer : register(b3)
{
    row_major float4x4 RealWorldMatrix;
}

struct VS_INPUT
{
    float3 position : POSITION; // Input position from vertex buffer
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};
struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float3 worldPosition : TEXCOORD0; 
    float3 normal : NORMAL0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 worldPosition = mul(float4(input.position, 1.0f), RealWorldMatrix);
    output.worldPosition = worldPosition.xyz;
    output.normal = mul(input.normal, (float3x3)RealWorldMatrix); // 로컬 노말 -> 월드 노말
    
    float tmp = worldPosition.y;
    worldPosition.y = worldPosition.z;
    worldPosition.z = tmp;
    
    tmp = output.normal.y;
    output.normal.y = output.normal.z;
    output.normal.z = tmp;
    
    output.position = mul(float4(input.position, 1.0f), WorldMatrix);
    output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    /*
    // 광원이 반대편에서 비추는 문제 발견
    // 모든 축 부호 반전 시도
    float3 transformedLight = float3(
        FireBallWorldPosition.x,
        FireBallWorldPosition.z,
        FireBallWorldPosition.y
    );
    
    float3 toLight = transformedLight - input.worldPosition;
    */
    float3 toLight = FireBallWorldPosition.xyz - input.worldPosition;
    
    // 거리 계산
    float distance = length(toLight);
    
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(toLight);
    float NdotL = dot(normal, lightDir);
    if(NdotL <= 0)
    {
        discard;
    }
    
    // Radius < distance 인 경우 그리지 않음
    if(FireBallParams.y < distance)
    {
        discard;
    }
    
    // 감쇠 계산: (1 - 거리/반경)
    float attenuation = saturate(1.0 - distance * FireBallParams.z);
    // 감쇠 곡선을 더 자연스럽게 위해 pow
    attenuation = pow(attenuation, FireBallParams.w);
    
    float3 finalColor = FireBallColor.rgb * FireBallParams.x * attenuation * NdotL;
    
    return float4(finalColor, 1.0);
}

