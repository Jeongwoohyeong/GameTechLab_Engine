#include "pch.h"
#include "Render/RenderPass/Public/LetterboxPass.h"
#include "Global/Function.h"
#include "Render/Renderer/Public/Pipeline.h"
#include "Render/Renderer/Public/RenderResourceFactory.h"
#include "Render/Renderer/Public/DeviceResources.h"
#include "Manager/UI/Public/ViewportManager.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"

/**
 * @brief 풀스크린 쿼드 렌더링을 위한 정점 구조체
 * Position: NDC 좌표 (-1 ~ 1)
 * UV: 텍스처 좌표 (0 ~ 1)
 */
struct FFullscreenVertex
{
	FVector2 Position; // NDC 공간 위치
	FVector2 UV;       // 텍스처 좌표
};

/**
 * @brief LetterboxPass 생성자
 * 풀스크린 쿼드와 상수 버퍼를 초기화합니다
 */
FLetterboxPass::FLetterboxPass(UPipeline* InPipeline,
	UDeviceResources* InDeviceResources,
	ID3D11VertexShader* InVS,
	ID3D11PixelShader* InPS,
	ID3D11InputLayout* InLayout,
	ID3D11SamplerState* InSampler)
	: FRenderPass(InPipeline, nullptr, nullptr)
	, DeviceResources(InDeviceResources)
	, VertexShader(InVS)
	, PixelShader(InPS)
	, InputLayout(InLayout)
	, SamplerState(InSampler)
{
	// 풀스크린 쿼드 버퍼 생성
	InitializeFullscreenQuad();

	// 레터박스 상수 버퍼 생성
	LetterboxConstantBuffer = FRenderResourceFactory::CreateConstantBuffer<FLetterboxConstants>();

	// 기본 종횡비 설정 (16:9)
	LetterboxParams.TargetAspectRatio = 16.0f / 9.0f;
}

/**
 * @brief LetterboxPass 소멸자
 */
FLetterboxPass::~FLetterboxPass()
{
	Release();
}

/**
 * @brief 레터박스 렌더링 패스를 실행합니다
 *
 * 실행 순서:
 * 1. 입력 텍스처 유효성 확인
 * 2. 상수 버퍼 업데이트 (종횡비, 렌더 타겟 크기)
 * 3. 렌더 타겟 설정 (OutputRTV 또는 백버퍼)
 * 4. 파이프라인 상태 설정
 * 5. 풀스크린 쿼드 렌더링
 * 6. 리소스 정리
 */
void FLetterboxPass::Execute(FRenderingContext& Context)
{
	// 입력 텍스처가 설정되지 않았으면 실행하지 않음
	if (!InputSRV)
	{
		UE_LOG("LetterboxPass: InputSRV가 설정되지 않았습니다");
		return;
	}

	// 상수 버퍼 업데이트 (현재 렌더 타겟 크기와 목표 종횡비)
	UpdateConstants();

	// 렌더 타겟 설정
	SetRenderTargets();

	// 파이프라인 정보 구성
	FPipelineInfo PipelineInfo = {};
	PipelineInfo.InputLayout = InputLayout;
	PipelineInfo.VertexShader = VertexShader;
	PipelineInfo.PixelShader = PixelShader;
	PipelineInfo.DepthStencilState = nullptr; // 깊이 테스트 불필요 (풀스크린 쿼드)
	PipelineInfo.BlendState = nullptr;        // 블렌딩 불필요
	PipelineInfo.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 파이프라인 업데이트
	Pipeline->UpdatePipeline(PipelineInfo);

	// 버퍼 바인딩
	UINT Offset = 0;
	Pipeline->SetVertexBuffer(FullscreenVertexBuffer, FullscreenStride);
	Pipeline->SetIndexBuffer(FullscreenIndexBuffer, 0);

	// 상수 버퍼, 텍스처, 샘플러 바인딩
	Pipeline->SetConstantBuffer(0, EShaderType::PS, LetterboxConstantBuffer);
	Pipeline->SetShaderResourceView(0, EShaderType::PS, InputSRV);
	Pipeline->SetSamplerState(0, EShaderType::PS, SamplerState);

	// 풀스크린 쿼드 렌더링 (2개의 삼각형 = 6개의 인덱스)
	Pipeline->DrawIndexed(FullscreenIndexCount, 0, 0);

	// 셰이더 리소스 언바인드 (다음 패스에서 RTV로 사용할 수 있도록)
	Pipeline->SetShaderResourceView(0, EShaderType::PS, nullptr);
}

/**
 * @brief LetterboxPass 리소스를 해제합니다
 */
void FLetterboxPass::Release()
{
	SafeRelease(FullscreenVertexBuffer);
	SafeRelease(FullscreenIndexBuffer);
	SafeRelease(LetterboxConstantBuffer);
}

/**
 * @brief 목표 종횡비를 설정합니다
 * @param AspectRatio 종횡비 값 (예: 2.333 = 21:9, 1.777 = 16:9, 2.39 = 시네마스코프)
 */
void FLetterboxPass::SetTargetAspectRatio(float AspectRatio)
{
	LetterboxParams.TargetAspectRatio = AspectRatio;
}

/**
 * @brief 입력 텍스처를 설정합니다
 * @param InInputSRV 입력 셰이더 리소스 뷰 (이전 포스트 프로세스 패스의 출력)
 */
void FLetterboxPass::SetInputTexture(ID3D11ShaderResourceView* InInputSRV)
{
	InputSRV = InInputSRV;
}

/**
 * @brief 출력 렌더 타겟을 설정합니다
 * @param InOutputRTV 출력 렌더 타겟 뷰 (nullptr이면 백버퍼로 렌더링)
 */
void FLetterboxPass::SetOutputRenderTarget(ID3D11RenderTargetView* InOutputRTV)
{
	OutputRTV = InOutputRTV;
}

void FLetterboxPass::SetFadeParameters(const FVector4& InColor, float InAmount)
{
	PendingFadeColor = InColor;
	PendingFadeAmount = Clamp(InAmount, 0.0f, 1.0f);
}

/**
 * @brief 풀스크린 쿼드를 초기화합니다
 *
 * NDC 공간에서 화면 전체를 덮는 사각형을 생성합니다.
 * - 좌측 상단: (-1, 1) → UV(0, 0)
 * - 우측 상단: (1, 1) → UV(1, 0)
 * - 우측 하단: (1, -1) → UV(1, 1)
 * - 좌측 하단: (-1, -1) → UV(0, 1)
 *
 * 2개의 삼각형으로 사각형을 구성합니다 (시계 반대 방향)
 */
void FLetterboxPass::InitializeFullscreenQuad()
{
	// 4개의 정점 (NDC 공간 좌표와 UV)
	static const FFullscreenVertex Vertices[] =
	{
		{{-1.f,  1.f}, {0.f, 0.f}}, // 좌측 상단
		{{ 1.f,  1.f}, {1.f, 0.f}}, // 우측 상단
		{{ 1.f, -1.f}, {1.f, 1.f}}, // 우측 하단
		{{-1.f, -1.f}, {0.f, 1.f}}, // 좌측 하단
	};

	// 6개의 인덱스 (2개의 삼각형)
	// 첫 번째 삼각형: 0-1-2 (좌상-우상-우하)
	// 두 번째 삼각형: 0-2-3 (좌상-우하-좌하)
	static const uint32 Indices[] = { 0, 1, 2, 0, 2, 3 };

	FullscreenStride = sizeof(FFullscreenVertex);
	FullscreenIndexCount = static_cast<UINT>(sizeof(Indices) / sizeof(Indices[0]));

	// 정점 버퍼 생성
	D3D11_BUFFER_DESC VertexBufferDesc = {};
	VertexBufferDesc.ByteWidth = sizeof(Vertices);
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 변경되지 않는 정적 버퍼
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA VertexBufferData = {};
	VertexBufferData.pSysMem = Vertices;

	HRESULT Result = DeviceResources->GetDevice()->CreateBuffer(&VertexBufferDesc, &VertexBufferData, &FullscreenVertexBuffer);
	if (FAILED(Result))
	{
		UE_LOG_ERROR("LetterboxPass: 풀스크린 정점 버퍼 생성 실패");
	}

	// 인덱스 버퍼 생성
	D3D11_BUFFER_DESC IndexBufferDesc = {};
	IndexBufferDesc.ByteWidth = sizeof(Indices);
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA IndexBufferData = {};
	IndexBufferData.pSysMem = Indices;

	Result = DeviceResources->GetDevice()->CreateBuffer(&IndexBufferDesc, &IndexBufferData, &FullscreenIndexBuffer);
	if (FAILED(Result))
	{
		UE_LOG_ERROR("LetterboxPass: 풀스크린 인덱스 버퍼 생성 실패");
	}
}

/**
 * @brief 레터박스 상수 버퍼를 업데이트합니다
 *
 * 현재 렌더 타겟의 크기와 설정된 목표 종횡비를
 * 상수 버퍼에 반영하여 GPU로 전달합니다.
 */
void FLetterboxPass::UpdateConstants()
{
	// ActiveViewportRect 크기를 기반으로 종횡비 계산
	FRect ActiveRect = UViewportManager::GetInstance().GetActiveViewportRect();

	// 렌더 타겟 크기를 상수 버퍼에 설정
	LetterboxParams.RenderTargetWidth = static_cast<float>(ActiveRect.Width);
	LetterboxParams.RenderTargetHeight = static_cast<float>(ActiveRect.Height);

	// PlayerCameraManager로부터 레터박스 알파값과 애니메이션 진행도 가져오기
	if (PlayerCameraManager)
	{
		LetterboxParams.LetterBoxAlpha = PlayerCameraManager->GetLetterBoxAlpha();
		LetterboxParams.LetterBoxAnimationProgress = PlayerCameraManager->GetLetterBoxAnimationProgress();
	}
	else
	{
		LetterboxParams.LetterBoxAlpha = 0.0f;
		LetterboxParams.LetterBoxAnimationProgress = 0.0f;
	}

	// SceneColor 텍스처 UV 매핑 계산
	// SceneColor 텍스처는 백버퍼 전체 크기이지만, 실제 렌더링은 ActiveViewportRect에만 됨
	const float BackbufferWidth = static_cast<float>(DeviceResources->GetWidth());
	const float BackbufferHeight = static_cast<float>(DeviceResources->GetHeight());

	LetterboxParams.ViewportUVOffsetX = static_cast<float>(ActiveRect.Left) / BackbufferWidth;
	LetterboxParams.ViewportUVOffsetY = static_cast<float>(ActiveRect.Top) / BackbufferHeight;
	LetterboxParams.ViewportUVScaleX = static_cast<float>(ActiveRect.Width) / BackbufferWidth;
	LetterboxParams.ViewportUVScaleY = static_cast<float>(ActiveRect.Height) / BackbufferHeight;

	LetterboxParams.FadeColor = PendingFadeColor;
	LetterboxParams.FadeAmount = PendingFadeAmount;

	// 상수 버퍼 업데이트 (GPU 메모리로 전송)
	FRenderResourceFactory::UpdateConstantBufferData(LetterboxConstantBuffer, LetterboxParams);
}

/**
 * @brief 렌더 타겟을 설정합니다
 *
 * OutputRTV가 설정되어 있으면 해당 타겟으로 렌더링하고,
 * 없으면 백버퍼(스왑체인)로 직접 렌더링합니다.
 *
 * 뷰포트는 ActiveViewportRect를 사용하여 위젯 영역을 제외한 부분에만 렌더링합니다.
 */
void FLetterboxPass::SetRenderTargets()
{
	// OutputRTV가 설정되어 있으면 사용, 없으면 백버퍼 사용
	ID3D11RenderTargetView* RenderTargetView = OutputRTV
		? OutputRTV
		: DeviceResources->GetRenderTargetView();

	// 렌더 타겟 설정 (깊이 버퍼 없음)
	DeviceResources->GetDeviceContext()->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	// 뷰포트 설정: ActiveViewportRect 기반 (위젯 제외 영역에만 출력)
	FRect ActiveRect = UViewportManager::GetInstance().GetActiveViewportRect();
	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = static_cast<float>(ActiveRect.Left);
	Viewport.TopLeftY = static_cast<float>(ActiveRect.Top);
	Viewport.Width = static_cast<float>(ActiveRect.Width);
	Viewport.Height = static_cast<float>(ActiveRect.Height);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	DeviceResources->GetDeviceContext()->RSSetViewports(1, &Viewport);
}

