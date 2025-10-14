cbuffer ScreenSizeBuffer : register(b0)
{
    float2 RcpScreenSize;
    // Edge 강조 모드와 FXAA 모드를 위한 int flag
    int Mode;
    int pad;
}

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

Texture2D SceneTexture : register(t0);
SamplerState DefaultSampler : register(s0);

#define FXAAMode 0
#define EdgeMode 1

PS_INPUT mainVS(uint VertexID : SV_VertexID)
{
    PS_INPUT Output;

    Output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);

    Output.Position = float4(Output.TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);    

    return Output;
}

float4 mainPS(PS_INPUT Input) : SV_Target
{
    float2 FinalTexCoord = Input.TexCoord;
    // 경계 판단의 최소 Luma Contrast값
    const float EdgeThreshold           = 0.166f;
    // 어두운 영역 노이즈 무시용 최소 Luma값
    const float EdgeThresholdMin        = 0.0833f;
    // 경계 끝 탐색의 밥복 회수
    const int EdgeSearchIteration       = 10;
    // 서브픽셀 블렌딩 강도
    const float BlendIntensity          = 0.75f;

    // Luma 계산을 위한 값
    const float3 LumaFactor = float3(0.299f, 0.587f, 0.114f);

    // 현재 픽셀 (main pixel)
    float3 ColorM = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0).rgb;
    float LumaM = dot(ColorM, LumaFactor);

    // 상(N), 하(S), 좌(W), 우(E) 픽셀 Luma
    float3 ColorN = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(0, -1)).rgb;
    float LumaN = dot(ColorN, LumaFactor);

    float3 ColorS = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(0, 1)).rgb;
    float LumaS = dot(ColorS, LumaFactor);

    float3 ColorW = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(-1, 0)).rgb;
    float LumaW = dot(ColorW, LumaFactor);

    float3 ColorE = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(1, 0)).rgb;
    float LumaE = dot(ColorE, LumaFactor);

    // 경계 존재 여부 검사
    float LumaMin = min(LumaM, min(min(LumaN, LumaS), min(LumaW, LumaE)));
    float LumaMax = max(LumaM, max(max(LumaN, LumaS), max(LumaW, LumaE)));
    float LumaRange = LumaMax - LumaMin;

    // Luma Contrast가 Threshold 미만이면 early return
    if (LumaRange < max(EdgeThresholdMin, LumaMax * EdgeThreshold))
    {
        return float4(ColorM, 1.0f);        
    }

    // 3x3 영역 Luma Sampling으로 경계 방향 정밀하게 판단
    float3 ColorNW = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(-1, -1)).rgb;
    float LumaNW = dot(ColorNW, LumaFactor);
    float3 ColorNE = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(1, -1)).rgb;
    float LumaNE = dot(ColorNE, LumaFactor);
    float3 ColorSW = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(-1, 1)).rgb;
    float LumaSW = dot(ColorSW, LumaFactor);
    float3 ColorSE = SceneTexture.SampleLevel(DefaultSampler, Input.TexCoord, 0, int2(1, 1)).rgb;
    float LumaSE = dot(ColorSE, LumaFactor);

    // 수평/수직 경계 검사 - sobel filter 이용
    float EdgeHorizontal = abs(LumaNW + 2 * LumaN + LumaNE - LumaSW - 2 * LumaS - LumaSE);
    float EdgeVertical = abs(LumaNW + 2 * LumaW + LumaSW - LumaNE - 2 * LumaE - LumaSE);
    if (Mode == FXAAMode)
    {
        bool bIsHorizontal = EdgeHorizontal >= EdgeVertical;

        float2 Step = bIsHorizontal ? float2(0.0f, RcpScreenSize.Y) : float2(RcpScreenSize.X, 0.0f);

        // Positive 방향
        float2 PositivePosition = Input.TexCoord + Step;
        // Negative 방향
        float2 NegativePosition = Input.TexCoord - Step;

        float PositiveLumaEnd = bIsHorizontal ? LumaS : LumaE;
        float NegativeLumaEnd = bIsHorizontal ? LumaN : LumaW;
        float PositiveGradient = PositiveLumaEnd - LumaM;
        float NegativeGradient = NegativeLumaEnd - LumaM;

        // Positive 방향 탐색
        for (int i = 0; i < EdgeSearchIteration; i++)
        {
            float3 PositiveColor = SceneTexture.SampleLevel(DefaultSampler, PositivePosition, 0).rgb;
            float PositiveLuma = dot(PositiveColor, LumaFactor) - LumaM;
            if ((PositiveLuma * PositiveGradient) < 0.0f)
            {
                PositivePosition -= Step;
                break;
            }
            PositivePosition += Step;
        }

        // Negative 방향 탐색
        for (int i = 0; i < EdgeSearchIteration; i++)
        {
            float3 NegativeColor = SceneTexture.SampleLevel(DefaultSampler, NegativePosition, 0).rgb;
            float NegativeLuma = dot(NegativeColor, LumaFactor) - LumaM;
            if ((NegativeLuma * NegativeGradient) < 0.0f)
            {
                NegativePosition -= Step;
                break;
            }
            NegativePosition -= Step;
        }

        // 서브 픽셀 블렌딩
        float NegativeDistance = bIsHorizontal
                                     ? (Input.TexCoord.y - NegativePosition.y)
                                     : (Input.TexCoord.x - NegativePosition.x);
        float PositiveDistance = bIsHorizontal
                                     ? (PositivePosition.y - Input.TexCoord.y)
                                     : (PositivePosition.x - Input.TexCoord.x);

        float Direction = (NegativeDistance < PositiveDistance) ? -1.0f : 1.0f;
        float Ratio = min(NegativeDistance, PositiveDistance) / (NegativeDistance + PositiveDistance);
        float PixelOffset = (Ratio - 0.5f) * BlendIntensity;
        FinalTexCoord += Direction * Step * PixelOffset;
    }
    else if (Mode == EdgeMode)
    {
        float EdgeStrength = saturate(EdgeHorizontal + EdgeVertical);

        return float4(0.0f, EdgeStrength, 0.0f, 1.0f);
    }
    

    return SceneTexture.SampleLevel(DefaultSampler, FinalTexCoord, 0);
}