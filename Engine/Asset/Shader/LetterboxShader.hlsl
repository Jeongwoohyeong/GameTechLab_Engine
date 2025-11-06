// ============================================================================
// Letterbox Shader
// 시네마틱 레터박스 효과를 위한 포스트 프로세싱 셰이더
// 상하단에 검은 바를 추가하여 목표 종횡비를 만들어냅니다
// ============================================================================

// 레터박스 상수 버퍼
cbuffer LetterboxParams : register(b0)
{
	float4 FadeColor;
    float FadeAmount;
    
    float TargetAspectRatio; // 목표 종횡비 (예: 21/9 = 2.333, 16/9 = 1.777)
    float RenderTargetWidth; // Active Viewport 너비
    float RenderTargetHeight; // Active Viewport 높이
    float LetterBoxAlpha; // 레터박스 알파 (0.0 = 완전 투명, 1.0 = 완전 불투명)

    // SceneColor 텍스처 UV 매핑 정보
    float ViewportUVOffsetX; // UV 오프셋 X
    float ViewportUVOffsetY; // UV 오프셋 Y
    float ViewportUVScaleX; // UV 스케일 X
    float ViewportUVScaleY; // UV 스케일 Y

    // 래터박스 애니메이션 파라미터
    float LetterBoxAnimationProgress; // 애니메이션 진행도 (0.0 = 시작, 1.0 = 완료)
    float Padding1;
    float Padding2;
    float Padding3;
}

// 입력 텍스처 (이전 포스트 프로세스 패스의 출력, 예: FXAA 결과)
Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

// Vertex Shader 입력 구조체
struct VSInput
{
    float2 Position : POSITION; // NDC 좌표 (-1 ~ 1)
    float2 TexCoord : TEXCOORD0; // UV 좌표 (0 ~ 1)
};

// Vertex Shader 출력 / Pixel Shader 입력 구조체
struct VSOutput
{
    float4 Position : SV_POSITION; // 화면 공간 위치
    float2 TexCoord : TEXCOORD0; // UV 좌표
};

// ============================================================================
// Vertex Shader
// 풀스크린 쿼드를 렌더링하기 위한 간단한 패스스루 VS
// ============================================================================
VSOutput mainVS(VSInput input)
{
    VSOutput output;
    // 입력 위치를 그대로 출력 위치로 변환 (Z=0, W=1로 NDC 공간에 배치)
    output.Position = float4(input.Position, 0.0f, 1.0f);
    output.TexCoord = input.TexCoord;
    return output;
}

// ============================================================================
// Pixel Shader
// 레터박스 효과를 적용하는 메인 로직
// ============================================================================
float4 mainPS(VSOutput input) : SV_Target
{
    float2 TexCoord = input.TexCoord;

    // 현재 렌더 타겟의 종횡비 계산 (Width / Height)
    float CurrentAspectRatio = RenderTargetWidth / RenderTargetHeight;

    // 레터박스 바의 최종 높이 계산
    // TargetAspectRatio가 CurrentAspectRatio보다 크면 상하단에 검은 바가 생김
    // 공식: (1.0 - CurrentAspect / TargetAspect) * 0.5
    float FinalBarHeight = (1.0 - (CurrentAspectRatio / TargetAspectRatio)) * 0.5;

    // BarHeight가 음수면 레터박스가 필요 없음 (목표 종횡비가 현재보다 좁음)
    FinalBarHeight = max(0.0, FinalBarHeight);

    // 애니메이션 진행도에 따라 현재 바 높이 계산
    // AnimationProgress = 0.0 → BarHeight = 0.0 (바가 안 보임)
    // AnimationProgress = 1.0 → BarHeight = FinalBarHeight (바가 완전히 내려옴)
    float AnimatedBarHeight = FinalBarHeight * LetterBoxAnimationProgress;

    // SceneColor 텍스처 UV 리매핑
    // SceneColor 텍스처는 백버퍼 전체 크기이지만, 실제 렌더링은 ActiveViewportRect에만 됨
    // TexCoord는 ActiveViewportRect 기준 (0~1)이므로, 백버퍼 텍스처의 올바른 영역으로 매핑
    float2 SceneColorUV;
    SceneColorUV.x = ViewportUVOffsetX + TexCoord.x * ViewportUVScaleX;
    SceneColorUV.y = ViewportUVOffsetY + TexCoord.y * ViewportUVScaleY;

    // 리매핑된 UV로 SceneColor 샘플링
    float4 SceneColor = InputTexture.Sample(InputSampler, SceneColorUV);
    float FadeAlpha = saturate(FadeAmount);
    SceneColor = lerp(SceneColor, FadeColor, FadeAlpha);

    // UV 좌표가 바 영역 안에 있는지 확인
    // UV.y가 [0, AnimatedBarHeight] 또는 [1-AnimatedBarHeight, 1] 범위에 있으면 검은 바 영역
    if (TexCoord.y < AnimatedBarHeight || TexCoord.y > (1.0 - AnimatedBarHeight))
    {
        // 검은색과 SceneColor를 알파값으로 블렌딩 (페이드 인/아웃 효과)
        // LetterBoxAlpha = 0.0 → SceneColor 그대로 (래터박스 없음)
        // LetterBoxAlpha = 1.0 → 완전히 검은색 (래터박스 완전히 보임)
        float3 BlackColor = float3(0.0, 0.0, 0.0);
        float3 BlendedColor = lerp(SceneColor.rgb, BlackColor, LetterBoxAlpha);
        return float4(BlendedColor, 1.0);
    }

    return SceneColor;
}

