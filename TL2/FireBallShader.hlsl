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
    
    float4 worldPosition = mul(float4(input.position, 1.0f), WorldMatrix);
    
    output.worldPosition = worldPosition.xyz;
    
    output.position = mul(worldPosition, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.normal = mul(input.normal, (float3x3) WorldMatrix); // 로컬 노말 -> 월드 노말
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // 픽셀의 worldPos to 빛의 위치 벡터
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

