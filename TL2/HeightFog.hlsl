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

cbuffer ViewportBuffer : register(b8)
{
    float4 ViewportRect; // Normalized [0,1]: x=StartX, y=StartY, z=SizeX, w=SizeY
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

// b10: Inverse matrix buffer (3 matrices: InvWorld, InvView, InvProj)
cbuffer InvMatrixBuffer : register(b10)
{
    matrix InvWorld;
    matrix InvView;
    matrix InvProj;
};

float ComputeHeightFog(float3 worldPos, float3 camPos)
{
    float3 rayDir = worldPos - camPos;
    float d = length(rayDir);
    float vh = rayDir.z / max(d, 1e-5); // z-axis component (instead of normalize)
    float h0 = camPos.z;
    
    float adjustedDistance = min(max(0.0f, d - StartDistance), FogCutoffDistance);
    float a = vh * FogHeightFalloff;
    
    float tau;
    if (abs(a) < 1e-5)
        tau = FogDensity * exp(-h0 * FogHeightFalloff) * adjustedDistance;
    else
        tau = FogDensity * exp(-h0 * FogHeightFalloff) * (1.0f - exp(-adjustedDistance * a)) / a;
    
    return 1.0f - exp(-tau);
}

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
    Out.texCoord = (pos[VertexID] + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f);
    Out.normal = float3(0.0f, 0.0f, 1.0f); // Default normal
    Out.color = float4(1.0f, 0.0f, 0.0f, 0.2f); // Default color
    
    return Out;
}

float4 mainPS(PS_INPUT In) : SV_TARGET
{
    // Step 1: Get viewport-corrected UV for depth sampling
    float2 depthUV = ViewportRect.xy + In.texCoord * ViewportRect.zw;

    // Step 2: Sample depth from depth buffer
    float depth = SceneDepthTexture.Sample(DefaultSampler, depthUV).r;

    // Step 3: Convert UV [0,1] to NDC [-1,1]
    float2 ndcXY;
    ndcXY.x = In.texCoord.x * 2.0f - 1.0f;      // [0,1] → [-1,1]
    ndcXY.y = 1.0f - In.texCoord.y * 2.0f; // [0,1] → [1,-1] (Y flip for DirectX)

    // Step 4: Construct clip space position (homogeneous coordinates)
    // IMPORTANT: Use raw depth value, not modified depth!
    float4 clipPos = float4(ndcXY, depth, 1.0f);

    // Step 5: Compute InvViewProj = InvView * InvProj (matrix multiplication in shader)
    matrix InvViewProj = mul(InvView, InvProj);

    // Step 6: Transform to world space using inverse ViewProj matrix
    float4 worldPos = mul(InvViewProj, clipPos);
    worldPos.xyz /= worldPos.w;  // Perspective division (critical!)

    // Step 7: Extract camera position from InvView matrix (last row/column depending on row/column-major)
    // In HLSL with row_major (default), camera position is in the 4th row
    float3 camPos = float3(InvView[0][3], InvView[1][3], InvView[2][3]);
    
    float fogFactor = ComputeHeightFog(worldPos.xyz, camPos);
    //return float4(1.0f, 0.0f, 0.0f, fogFactor);
    return float4(FogInscatteringColor.xyz, FogInscatteringColor.w * fogFactor);
}