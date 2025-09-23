#pragma once
#include "DeviceResources.h"
#include "Core/Object.h"
#include "Components/SceneComponent.h"
#include "Editor/EditorPrimitive.h"
#include "Render/UI/Layout/SWindow.h"
#include "Render/UI/Layout/MultiViewBuilders.h"
#include "Editor/Camera.h"

class UPipeline;
class UDeviceResources;
class UPrimitiveComponent;
class UStaticMesh;
class AActor;
class AGizmo;
class UEditor;
struct FPipelineInfo;
/**
 * @brief Rendering Pipeline 전반을 처리하는 클래스
 *
 * Direct3D 11 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 포인터들
 * @param Device GPU와 통신하기 위한 Direct3D 장치
 * @param DeviceContext GPU 명령 실행을 담당하는 컨텍스트
 * @param SwapChain 프레임 버퍼를 교체하는 데 사용되는 스왑 체인
 *
 * // 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
 * @param FrameBuffer 화면 출력용 텍스처
 * @param FrameBufferRTV 텍스처를 렌더 타겟으로 사용하는 뷰
 * @param RasterizerState 래스터라이저 상태(컬링, 채우기 모드 등 정의)
 * @param ConstantBuffer 쉐이더에 데이터를 전달하기 위한 상수 버퍼
 *
 * @param ClearColor 화면을 초기화(clear)할 때 사용할 색상 (RGBA)
 * @param ViewportInfo 렌더링 영역을 정의하는 뷰포트 정보
 *
 * @param DefaultVertexShader
 * @param DefaultPixelShader
 * @param DefaultInputLayout
 * @param Stride
 *
 * @param vertexBufferSphere
 * @param numVerticesSphere
 */
class URenderer : public UObject
{
	DECLARE_CLASS(URenderer, UObject)
	DECLARE_SINGLETON(URenderer)

public:
	// Viewport layout modes
	enum class EViewportLayout : uint8 { Single = 0, Quad = 1 };
	enum class EViewportType : uint8
	{
		Perspective,
		Top,
		Right,
		Front
	};

public:
	void Init(HWND InWindowHandle);
	void Release();

	void CreateConstantBuffer();

	static void ReleaseVertexBuffer(ID3D11Buffer* InVertexBuffer);
	void ReleaseConstantBuffer();
	void ReleaseResource();
	void ReleaseInstanceBuffer();

	void Update(UEditor* Editor);
	void RenderBegin();

// Layout control
	inline void ToggleViewportLayout() { SetViewportLayout(CurrentLayout == EViewportLayout::Quad ? EViewportLayout::Single : EViewportLayout::Quad); }
inline void SetViewportLayout(EViewportLayout InLayout)
	{
		if (CurrentLayout == InLayout) return;
		if (InLayout == EViewportLayout::Quad)
		{
			// 즉시 2x2 트리 생성하여 확실하게 멀티뷰 진입
			if (!MultiViewRoot)
			{
				MultiViewRoot = BuildQuadView(nullptr, nullptr, nullptr, nullptr);
			}
			CurrentLayout = EViewportLayout::Quad;
		}
		else
		{
			// 단일뷰 전환 시 트리 제거
			if (MultiViewRoot)
			{
				delete MultiViewRoot;
				MultiViewRoot = nullptr;
			}
			CurrentLayout = EViewportLayout::Single;
		}
	}
	EViewportLayout GetViewportLayout() const { return CurrentLayout; }
	void RenderLevel();
	void RenderText(const FVector& CameraLocation);
	void RenderEditorPrimitive(FEditorPrimitive& Primitive, const FPipelineDescKey PipelineDescKey);
	void RenderEnd() const;


	void OnResize(uint32 Inwidth = 0, uint32 InHeight = 0);
	bool GetIsResizing() { return bIsResizing; }
	void SetIsResizing(bool isResizing) { bIsResizing = isResizing; }

	// Viewport layout save/load (editor.ini)
	void LoadViewportLayout();
	void SaveViewportLayout() const;


	template<typename T>
	ID3D11Buffer* CreateVertexBuffer(TArray<T>& InVertices) const
	{
		D3D11_BUFFER_DESC VertexBufferDesc = {};
		VertexBufferDesc.ByteWidth = InVertices.size() * sizeof(T);
		VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
		VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA VertexBufferSRD = { InVertices.data() };

		ID3D11Buffer* VertexBuffer;

		GetDevice()->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, &VertexBuffer);

		return VertexBuffer;
	}

	template<typename T>
	ID3D11Buffer* CreateIndexBuffer(TArray<T>& InIndices) const
	{
		D3D11_BUFFER_DESC IndexBufferDesc = {};
		IndexBufferDesc.ByteWidth = InIndices.size() * sizeof(T);
		IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
		IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA IndexBufferSRD = { InIndices.data() };

		ID3D11Buffer* IndexBuffer;

		GetDevice()->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, &IndexBuffer);

		return IndexBuffer;
	}

	void CreateInstanceBuffer();

	void UpdateConstant(const FMatrix& InMatrix) const;
	void UpdateConstant(const FVector& InPosition, const FVector& InRotation, const FVector& InScale) const;
	void UpdateConstant(const FViewProjConstants& InViewProjConstants) const;
	void UpdateConstant(const FVector4& Color) const;
	void UpdateInstance(const TArray<FTextInstance>* Instance);

	void SetViewMode(EViewModeIndex InViewMode) { CurrentViewMode = InViewMode; }
	EViewModeIndex GetViewMode(EViewModeIndex InViewMode) const { return CurrentViewMode; }

	/** Show Flags management */
	void SetShowFlags(EEngineShowFlags InShowFlags) { CurrentShowFlags = InShowFlags; }
	EEngineShowFlags GetShowFlags() const { return CurrentShowFlags; }
	void ToggleShowFlag(EEngineShowFlags InFlag) { CurrentShowFlags = CurrentShowFlags ^ InFlag; }
	bool IsShowFlagEnabled(EEngineShowFlags InFlag) const { return HasFlag(CurrentShowFlags, InFlag); }

	ID3D11Device* GetDevice() const { return DeviceResources->GetDevice(); }
	ID3D11DeviceContext* GetDeviceContext() const { return DeviceResources->GetDeviceContext(); }
	IDXGISwapChain* GetSwapChain() const { return DeviceResources->GetSwapChain(); }
	ID3D11RenderTargetView* GetRenderTargetView() const { return DeviceResources->GetRenderTargetView(); }
	UDeviceResources* GetDeviceResources() const { return DeviceResources; }

	/** LineBatchRenderer에서 사용할 공개 메서드 */
	UPipeline* GetPipeline() const { return Pipeline; }

private:
	UPipeline* Pipeline = nullptr;
	UDeviceResources* DeviceResources = nullptr;
	EViewModeIndex CurrentViewMode = EViewModeIndex::Lit;
	EEngineShowFlags CurrentShowFlags = EEngineShowFlags::SF_Default;
	TArray<UPrimitiveComponent*> PrimitiveComponents;

	// Multiview root splitter
	SWindow* MultiViewRoot = nullptr;
	EViewportLayout CurrentLayout = EViewportLayout::Single; // default: single view on start

	// Per-viewport cameras and types
	UCamera* ViewCameras[4] = { nullptr, nullptr, nullptr, nullptr };
	EViewportType ViewTypes[4] = { EViewportType::Perspective, EViewportType::Top, EViewportType::Right, EViewportType::Front };

private:
	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	ID3D11DepthStencilState* DisabledDepthStencilState = nullptr;
	ID3D11DepthStencilState* TextDepthStencilState = nullptr;
	ID3D11BlendState* TextBlendState = nullptr;
	ID3D11Buffer* ConstantBufferModels = nullptr;
	ID3D11Buffer* ConstantBufferPerFrame = nullptr;
	ID3D11Buffer* ConstantBufferColor = nullptr;
	ID3D11Buffer* ConstantBufferCharTable = nullptr;

	ID3D11Buffer* TextInstanceBuffer = nullptr;
	/////////////////////////////////////
	FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };



	uint32 StrideStaticMesh = sizeof(FNormalVertex);

	uint32 Stride = sizeof(FVertex);
	uint32 StrideTextVertex = sizeof(FTextVertex);
	uint32 StrideTextInstance = sizeof(FTextInstance);



	struct FStructuredBufferResource
	{
		ID3D11Buffer* Buffer = nullptr;
		ID3D11ShaderResourceView* ShaderResourceView = nullptr;	//버퍼에 대한 뷰
		uint32 Capacity = 0;	//스트럭처드 버퍼 크기
	};

	TMap<UStaticMesh*, FStructuredBufferResource> StaticMeshStructuredBuffers;
private:

	FStructuredBufferResource& GetOrCreateStructuredBuffer(UStaticMesh* InKey);
	void EnsureStructuredBufferCapacity(FStructuredBufferResource& InResource, uint32 InRequiredInstanceCount);
	void UploadStructuredBufferData(FStructuredBufferResource& InResource, const void* InData, uint32 InInstanceCount);
	void ReleasePrimitiveInstanceBuffers();

	bool bIsResizing = false;
};
