cbuffer ScreenSizeBuffer : register(b0)
{
    float4 ViewportRect;
    // Edge 강조 모드와 FXAA 모드를 위한 int flag
    int Mode;
    float3 Pad;
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
#define LumaFactor float3(0.299f, 0.587f, 0.114f)

float Luma(float3 RGB)
{
    return dot(RGB, LumaFactor);
}

PS_INPUT mainVS(uint VertexID : SV_VertexID)
{
    PS_INPUT Output;

    Output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);

    Output.Position = float4(Output.TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);    

    return Output;
}

float4 mainPS(PS_INPUT Input) : SV_Target
{
    uint Width, Height;
    SceneTexture.GetDimensions(Width, Height);
    float2 SceneSize = float2(Width, Height);
    float2 RcpScreenSize = 1.0f / SceneSize / ViewportRect.zw;
    float2 ViewportTexCoord = ViewportRect.xy + Input.TexCoord * ViewportRect.zw;
    ViewportTexCoord /= SceneSize;
    //return float4(SceneTexture.Sample(DefaultSampler, ViewportTexCoord / SceneSize).rgb, 1.0f);

    float2 FinalTexCoord = ViewportTexCoord;
    // 경계 판단의 최소 Luma Contrast값
    const float EdgeThreshold           = 0.125f;
    // 어두운 영역 노이즈 무시용 최소 Luma값
    const float EdgeThresholdMin        = 0.0312f;
    // 서브픽셀 블렌딩 강도
    const float BlendIntensity          = 0.5f;

    // 픽셀 컬러
    float3 ColorM = SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0).rgb;    
    float3 ColorN = SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(0, -1)).rgb;
    float3 ColorS = SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(0, 1)).rgb;
    float3 ColorW = SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(-1, 0)).rgb;
    float3 ColorE = SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(1, 0)).rgb;
    
    // 픽셀 Luma
    float LumaM = Luma(ColorM);    
    float LumaN = Luma(ColorN);
    float LumaS = Luma(ColorS);
    float LumaW = Luma(ColorW);
    float LumaE = Luma(ColorE);
    
    // 경계 존재 여부 검사
    float LumaMin = min(LumaM, min(min(LumaN, LumaS), min(LumaW, LumaE)));
    float LumaMax = max(LumaM, max(max(LumaN, LumaS), max(LumaW, LumaE)));
    float LumaContrast = LumaMax - LumaMin;

    float3 Average = (ColorN + ColorS + ColorW + ColorE) * 0.25f;
    
    // Luma Contrast가 Threshold 미만이면 early return
    if (LumaContrast < max(EdgeThresholdMin, LumaMax * EdgeThreshold))
    {        
        float3 Smooth = lerp(ColorM, Average, BlendIntensity);
        return float4(Smooth, 1.0f);        
    }
    
    // 3x3 영역 Luma Sampling으로 경계 방향 정밀하게 판단    
    float LumaNW = Luma(SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(-1, -1)).rgb);
    float LumaNE = Luma(SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(1, -1)).rgb);    
    float LumaSW = Luma(SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(-1, 1)).rgb);    
    float LumaSE = Luma(SceneTexture.SampleLevel(DefaultSampler, ViewportTexCoord, 0, int2(1, 1)).rgb);
    
    // 수평/수직 경계 검사 - sobel filter 이용
    float EdgeHorizontal = abs(LumaNW + 2 * LumaN + LumaNE - LumaSW - 2 * LumaS - LumaSE);
    float EdgeVertical = abs(LumaNW + 2 * LumaW + LumaSW - LumaNE - 2 * LumaE - LumaSE);
    if (Mode == FXAAMode)
    {
        // 경계 방향 벡터 계산
        float2 Direction;
        Direction.x = -((LumaNW + LumaNE) - (LumaSW + LumaSE));
        Direction.y = ((LumaNW + LumaSW) - (LumaNE + LumaSE));

        // 방향 벡터 안정화 및 UV 공간으로 전환 - 공부
        float DirectionReduce = max((LumaN + LumaS + LumaW + LumaE) * (0.25 * 0.125), 1.0f / 128.0f);
        float RcpDirMin = 1.0f / (min(abs(Direction.x), abs(Direction.y)) + DirectionReduce);
        Direction = clamp(Direction * RcpDirMin, -8.0f, 8.0f) * RcpScreenSize;

        // 서브 픽셀 샘플링 - 공부
        float3 RgbA = 0.5 *
            (SceneTexture.Sample(DefaultSampler, ViewportTexCoord + Direction * (1.0 / 3.0 - 0.5)).rgb +
            SceneTexture.Sample(DefaultSampler, ViewportTexCoord + Direction * (2.0 / 3.0 - 0.5)).rgb);

        float3 RgbB = RgbA * 0.5 + 0.25 * (
            SceneTexture.Sample(DefaultSampler, ViewportTexCoord + Direction * -0.5).rgb +
            SceneTexture.Sample(DefaultSampler, ViewportTexCoord + Direction * 0.5).rgb);

        // 과 블러 방지
        float LumaB = Luma(RgbB);
        float3 Result = (LumaB < LumaMin || LumaB > LumaMax) ? RgbA : RgbB;

        // 강한 경계에 미세한 블렌딩 추가
        Result = lerp(Result, Average, saturate(BlendIntensity) * 0.25f);

        return float4(Result, 1.0f);        
    }
    else if (Mode == EdgeMode)
    {
        float EdgeStrength = saturate(EdgeHorizontal + EdgeVertical);
    
        return float4(0.0f, 1.0f, 0.0f, 1.0f);
    }

    // 여기까지 도달하면 잘못된 모드 입력된거임
    return float4(1, 0, 0, 1);    
}