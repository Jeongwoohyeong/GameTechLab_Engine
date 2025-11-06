#pragma once
#include "Global/Vector.h"
#include "Render/RenderPass/Public/RenderPass.h"

class UDeviceResources;

/**
 * @brief 레터박스 상수 버퍼 구조체
 * 셰이더에 전달될 레터박스 파라미터들을 포함합니다
 * 16바이트 정렬을 위해 alignas(16)을 사용합니다
 */
struct alignas(16) FLetterboxConstants
{
	// Fade Properties
	FVector4 FadeColor = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
	float FadeAmount = 0.0f;

	float TargetAspectRatio = 16.0f / 9.0f; // 목표 종횡비 (기본값: 16:9)
	float RenderTargetWidth = 1920.0f;      // Active Viewport 너비
	float RenderTargetHeight = 1080.0f;     // Active Viewport 높이
	float Padding = 0.0f;

	// SceneColor 텍스처 UV 매핑을 위한 정보
	float ViewportUVOffsetX = 0.0f;         // UV 오프셋 X (ActiveRect.Left / BackbufferWidth)
	float ViewportUVOffsetY = 0.0f;         // UV 오프셋 Y (ActiveRect.Top / BackbufferHeight)
	float ViewportUVScaleX = 1.0f;          // UV 스케일 X (ActiveRect.Width / BackbufferWidth)
	float ViewportUVScaleY = 1.0f;          // UV 스케일 Y (ActiveRect.Height / BackbufferHeight)
	float Padding2[3] = { 0.0f, 0.0f, 0.0f};
};

/**
 * @brief 레터박스 포스트 프로세싱 패스
 *
 * 시네마틱 레터박스 효과를 구현하는 렌더 패스입니다.
 * 입력 텍스처에 상하단 검은 바를 추가하여 목표 종횡비를 만들어냅니다.
 *
 * 주요 기능:
 * - 런타임에 종횡비 조정 가능 (21:9, 16:9, 2.39:1 등)
 * - 핑퐁 렌더링 지원 (입력/출력 텍스처 분리)
 * - 풀스크린 쿼드 렌더링 사용
 */
class FLetterboxPass : public FRenderPass
{
public:
	/**
	 * @brief LetterboxPass 클래스의 생성자입니다.
	 * @param InPipeline 파이프라인 객체입니다.
	 * @param InDeviceResources 디바이스 리소스 객체입니다.
	 * @param InVS 정점 셰이더입니다.
	 * @param InPS 픽셀 셰이더입니다.
	 * @param InLayout 입력 레이아웃입니다.
	 * @param InSampler 샘플러 상태입니다.
	 */
	FLetterboxPass(UPipeline* InPipeline,
	               UDeviceResources* InDeviceResources,
	               ID3D11VertexShader* InVS,
	               ID3D11PixelShader* InPS,
	               ID3D11InputLayout* InLayout,
	               ID3D11SamplerState* InSampler);

	/**
	 * @brief LetterboxPass 클래스의 소멸자입니다.
	 */
	~FLetterboxPass();

	/**
	 * @brief 레터박스 렌더링 패스를 실행합니다.
	 * @param Context 렌더링 컨텍스트입니다.
	 */
	void Execute(FRenderingContext& Context) override;

	/**
	 * @brief LetterboxPass에서 사용된 리소스를 해제합니다.
	 */
	void Release() override;

	/**
	 * @brief 목표 종횡비를 설정합니다.
	 * @param AspectRatio 종횡비 값 (예: 21/9 = 2.333, 16/9 = 1.777)
	 */
	void SetTargetAspectRatio(float AspectRatio);

	/**
	 * @brief 입력 텍스처(SRV)를 설정합니다.
	 * @param InInputSRV 입력 셰이더 리소스 뷰
	 */
	void SetInputTexture(ID3D11ShaderResourceView* InInputSRV);

	/**
	 * @brief 출력 렌더 타겟(RTV)을 설정합니다.
	 * @param InOutputRTV 출력 렌더 타겟 뷰
	 */
	void SetOutputRenderTarget(ID3D11RenderTargetView* InOutputRTV);

	// Setter 메서드들
	void SetVertexShader(ID3D11VertexShader* InVS) { VertexShader = InVS; }
	void SetPixelShader(ID3D11PixelShader* InPS) { PixelShader = InPS; }
	void SetInputLayout(ID3D11InputLayout* InLayout) { InputLayout = InLayout; }
	void SetSamplerState(ID3D11SamplerState* InSampler) { SamplerState = InSampler; }

	void SetFadeParameters(const FVector4& InColor, float InAmount);
	

private:
	/**
	 * @brief 풀스크린 쿼드(사각형)를 초기화합니다.
	 * 화면 전체를 덮는 삼각형 2개(사각형 1개)를 생성합니다.
	 */
	void InitializeFullscreenQuad();

	/**
	 * @brief 레터박스 상수 버퍼를 업데이트합니다.
	 * 현재 렌더 타겟 크기와 목표 종횡비를 상수 버퍼에 반영합니다.
	 */
	void UpdateConstants();

	/**
	 * @brief 렌더 타겟을 설정합니다.
	 * OutputRTV가 설정되어 있으면 해당 타겟으로, 없으면 백버퍼로 렌더링합니다.
	 */
	void SetRenderTargets();

private:
	// DeviceResources 포인터 (렌더 타겟 및 디바이스 컨텍스트 접근용)
	UDeviceResources* DeviceResources = nullptr;

	// 셰이더 리소스들
	ID3D11VertexShader* VertexShader = nullptr;
	ID3D11PixelShader* PixelShader = nullptr;
	ID3D11InputLayout* InputLayout = nullptr;
	ID3D11SamplerState* SamplerState = nullptr;

	// 풀스크린 쿼드 버퍼들
	ID3D11Buffer* FullscreenVertexBuffer = nullptr; // 정점 버퍼
	ID3D11Buffer* FullscreenIndexBuffer = nullptr;  // 인덱스 버퍼
	UINT FullscreenStride = 0;                      // 정점 크기
	UINT FullscreenIndexCount = 0;                  // 인덱스 개수

	// 레터박스 상수 버퍼
	ID3D11Buffer* LetterboxConstantBuffer = nullptr;
	FLetterboxConstants LetterboxParams{};

	// 입력/출력 텍스처
	ID3D11ShaderResourceView* InputSRV = nullptr;   // 입력 텍스처 (이전 패스의 출력)
	ID3D11RenderTargetView* OutputRTV = nullptr;    // 출력 렌더 타겟 (다음 패스의 입력 또는 백버퍼)

	FVector4 PendingFadeColor = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
	float PendingFadeAmount = 0.0f;
};

