#include "pch.h"
#include "Render/Renderer/Renderer.h"

#include "Level/Level.h"

#include "Manager/Level/LevelManager.h"
#include "Manager/UI/UIManager.h"
#include "Actor/Actor.h"
#include "Render/Renderer/Pipeline.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Editor/Editor.h"
#include "Components/TextComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/Material.h"
#include "Render/UI/Layout/MultiViewBuilders.h"
#include "Render/UI/Layout/SSplitter.h"
#include "Manager/Path/PathManager.h"
#include "Manager/Input/InputManager.h"
#include "ImGui/imgui.h"
#include <algorithm>

namespace
{
	struct FInstanceGPUData
	{
		FMatrix World;
		FVector4 Color;
	};
}

IMPLEMENT_CLASS(URenderer, UObject)
IMPLEMENT_SINGLETON(URenderer)

URenderer::URenderer() = default;

URenderer::~URenderer() = default;

void URenderer::Init(HWND InWindowHandle)
{
	DeviceResources = new UDeviceResources(InWindowHandle);
	Pipeline = new UPipeline(GetDeviceContext(), GetDevice());

	CreateInstanceBuffer();

	CreateConstantBuffer();

	/** LineBatchRenderer 초기화 */
	ULineBatchRenderer::GetInstance().Init();

	// Start in single-view layout; multiview is built on demand (F2 toggle)
	MultiViewRoot = nullptr;
	CurrentLayout = EViewportLayout::Single;

	// Create per-viewport cameras
	for (int i = 0; i < 4; ++i)
	{
		ViewCameras[i] = new UCamera();
	}
	ViewCameras[0]->SetCameraType(ECameraType::ECT_Perspective);
	ViewCameras[1]->SetCameraType(ECameraType::ECT_Orthographic); // Top
	ViewCameras[2]->SetCameraType(ECameraType::ECT_Orthographic); // Right
	ViewCameras[3]->SetCameraType(ECameraType::ECT_Orthographic); // Front
	// 오쏘 월드 단위 폭(씬 스케일에 따라 조정)
	// Default ortho widths: make bottom views closer by default
	// Tighter default ortho widths (closer zoom)
	ViewCameras[1]->SetOrthoWorldWidth(30.f);  // Top-Right (Top view)
	ViewCameras[1]->SetLocation(FVector(0, 30, 0));
	ViewCameras[1]->SetRotation(FVector(0, -90, 0));
	ViewCameras[1]->SetNearZ(0.1f); ViewCameras[1]->SetFarZ(1000.f);
	ViewCameras[2]->SetOrthoWorldWidth(30.f);  // Bottom-Left (Right view)
	ViewCameras[2]->SetLocation(FVector(0, 0, 30));
	ViewCameras[2]->SetRotation(FVector(90, 0, 0));
	ViewCameras[2]->SetNearZ(0.1f); ViewCameras[2]->SetFarZ(1000.f);
	ViewCameras[3]->SetOrthoWorldWidth(30.f);
	ViewCameras[3]->SetLocation(FVector(-30, 0, 0));
	ViewCameras[3]->SetRotation(FVector(0, 0, 0));
	ViewCameras[3]->SetNearZ(0.1f); ViewCameras[3]->SetFarZ(1000.f);

	// 강제 단일뷰로 시작 보장
	SetViewportLayout(EViewportLayout::Single);

	// Load layout from editor.ini
	LoadViewportLayout();
}

void URenderer::Release()
{
	/** LineBatchRenderer 해제 */
	ULineBatchRenderer::GetInstance().Release();

	// Save current layout to editor.ini
	SaveViewportLayout();

	ReleaseConstantBuffer();

	ReleaseResource();
	ReleaseInstanceBuffer();

	if (MultiViewRoot)
	{
		delete MultiViewRoot;
		MultiViewRoot = nullptr;
	}

	for (int i = 0; i < 4; ++i)
	{
		SafeDelete(ViewCameras[i]);
	}

	SafeDelete(Pipeline);
	SafeDelete(DeviceResources);
}


/**
 * @brief 렌더러에 사용된 모든 리소스를 해제하는 함수
 */
void URenderer::ReleaseResource()
{

	/** 렌더 타겟을 초기화 */
	if (GetDeviceContext())
	{
		GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);
	}
}



namespace
{
	static void CollectLeafRects(SWindow* Node, TArray<FRect>& Out)
	{
		if (!Node) return;
		if (Node->IsSplitter())
		{
			SSplitter* Split = static_cast<SSplitter*>(Node);
			CollectLeafRects(Split->GetLT(), Out);
			CollectLeafRects(Split->GetRB(), Out);
		}
		else
		{
			Out.Add(Node->GetRect());
		}
	}
}

void URenderer::Update(UEditor* Editor)
{
	RenderBegin();
	
	// 레이아웃 강제 보정: Single이면 트리 제거, Quad이면 필요 시 트리 생성
	if (CurrentLayout == EViewportLayout::Single)
	{
		if (MultiViewRoot) { delete MultiViewRoot; MultiViewRoot = nullptr; }
	}
	else if (CurrentLayout == EViewportLayout::Quad)
	{
		// ratio-based layout; tree not required
		UpdateSplitDrag();
	}
	ViewCameras[1]->SetOrthoWorldWidth(OrthoWidthConst *(VerticalRatio * 2 ));
	for (int Index = 2; Index < 4;Index++)
	{
		ViewCameras[Index]->SetOrthoWorldWidth(OrthoWidthConst * (2 - VerticalRatio * 2));
	}

	D3D11_VIEWPORT LocalViewports[4] = {};
	uint32 NumViewports = 0;

	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	GetSwapChain()->GetDesc(&swapchaindesc);

	ViewCameras[0]->SetLocation(Editor->GetCameraLocation());
	ViewCameras[0]->SetRotation(Editor->GetCamera().GetRotation());

	if (CurrentLayout == EViewportLayout::Quad)
	{
		float W = (float)swapchaindesc.BufferDesc.Width;
		float H = (float)swapchaindesc.BufferDesc.Height;
		float splitX = std::clamp(VerticalRatio, 0.1f, 0.9f) * W;
		float splitY = std::clamp(HorizontalRatio, 0.1f, 0.9f) * H;
		auto VP = [](float x, float y, float w, float h){ D3D11_VIEWPORT v = { x, y, std::max(2.0f, w), std::max(2.0f, h), 0.0f, 1.0f }; return v; };
		LocalViewports[0] = VP(0.0f,    0.0f,    splitX,       splitY);       // TL
		LocalViewports[1] = VP(0.0f,    splitY,  splitX,       H - splitY);   // BL
		LocalViewports[2] = VP(splitX,  0.0f,    W - splitX,   splitY);       // TR
		LocalViewports[3] = VP(splitX,  splitY,  W - splitX,   H - splitY);   // BR
		NumViewports = 4;
	}
	else // Single view
	{
		LocalViewports[0] = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
		NumViewports = 1;
	}

	for (uint32 i = 0; i < NumViewports; ++i)
	{
		GetDeviceContext()->RSSetViewports(1, &LocalViewports[i]);
		// 뷰포트 대응 스키서(Rect) 지정
		{
			// Robust scissor: clamp to swapchain bounds and ensure positive area
			DXGI_SWAP_CHAIN_DESC scd = {};
			GetSwapChain()->GetDesc(&scd);
			LONG maxW = (LONG)scd.BufferDesc.Width;
			LONG maxH = (LONG)scd.BufferDesc.Height;
			LONG left   = (LONG)LocalViewports[i].TopLeftX;
			LONG top    = (LONG)LocalViewports[i].TopLeftY;
			LONG right  = (LONG)(LocalViewports[i].TopLeftX + LocalViewports[i].Width);
			LONG bottom = (LONG)(LocalViewports[i].TopLeftY + LocalViewports[i].Height);
			left   = std::max<LONG>(0, std::min<LONG>(left,   maxW - 1));
			top    = std::max<LONG>(0, std::min<LONG>(top,    maxH - 1));
			right  = std::max<LONG>(left + 1, std::min<LONG>(right,  maxW));
			bottom = std::max<LONG>(top + 1,  std::min<LONG>(bottom, maxH));
			D3D11_RECT r{ left, top, right, bottom };
			GetDeviceContext()->RSSetScissorRects(1, &r);
		}
		// 뷰포트별 깊이 클리어(서브패스 간 간섭 방지)
		GetDeviceContext()->ClearDepthStencilView(DeviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		// RTV/DSV 바인딩 보강
		{
			ID3D11RenderTargetView* rtv = DeviceResources->GetRenderTargetView();
			ID3D11RenderTargetView* rtvs[] = { rtv };
			GetDeviceContext()->OMSetRenderTargets(1, rtvs, DeviceResources->GetDepthStencilView());
		}

		// Select camera per viewport
		UCamera* Cam = (CurrentLayout == EViewportLayout::Single) ? &Editor->GetCamera() : ViewCameras[i];

		// Update aspect from viewport size and upload constants
		if (Cam)
		{
			Cam->SetAspect(LocalViewports[i].Width / LocalViewports[i].Height);
			if (Cam->GetCameraType() == ECameraType::ECT_Perspective) Cam->UpdateMatrixByPers(); else Cam->UpdateMatrixByOrth();
			UpdateConstant(Cam->GetFViewProjConstants());
		}

		RenderLevel();
		Editor->RenderEditorBatched(Cam ? Cam->GetLocation() : Editor->GetCameraLocation());
		// For text sorting, use per-viewport camera location if available
		RenderText(Cam ? Cam->GetLocation() : Editor->GetCameraLocation());
	}

	// Reset viewport/scissor to full window before UI/overlay pass
	{
		DXGI_SWAP_CHAIN_DESC scd = {};
		GetSwapChain()->GetDesc(&scd);

		D3D11_VIEWPORT fullVp = {};
		fullVp.TopLeftX = 0.0f;
		fullVp.TopLeftY = 0.0f;
		fullVp.Width    = (float)scd.BufferDesc.Width;
		fullVp.Height   = (float)scd.BufferDesc.Height;
		fullVp.MinDepth = 0.0f;
		fullVp.MaxDepth = 1.0f;
		GetDeviceContext()->RSSetViewports(1, &fullVp);

		D3D11_RECT fullScissor;
		fullScissor.left   = 0;
		fullScissor.top    = 0;
		fullScissor.right  = (LONG)scd.BufferDesc.Width;
		fullScissor.bottom = (LONG)scd.BufferDesc.Height;
		GetDeviceContext()->RSSetScissorRects(1, &fullScissor);

		// Re-bind RTV/DSV for safety
		ID3D11RenderTargetView* rtv = DeviceResources->GetRenderTargetView();
		ID3D11RenderTargetView* rtvs[] = { rtv };
		GetDeviceContext()->OMSetRenderTargets(1, rtvs, DeviceResources->GetDepthStencilView());
	}

	UUIManager::GetInstance().Render();

	RenderEnd();
}


/**
 * @brief Render Prepare Step
 */
void URenderer::RenderBegin()
{
	ID3D11RenderTargetView* RenderTargetView = DeviceResources->GetRenderTargetView();
	GetDeviceContext()->ClearRenderTargetView(RenderTargetView, ClearColor);
	ID3D11DepthStencilView* DepthStencilView = DeviceResources->GetDepthStencilView();
	GetDeviceContext()->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	GetDeviceContext()->RSSetViewports(1, &DeviceResources->GetViewportInfo());

	ID3D11RenderTargetView* RenderTargetViews[] = { RenderTargetView }; // 배열 생성

	GetDeviceContext()->OMSetRenderTargets(1, RenderTargetViews, DeviceResources->GetDepthStencilView());
	DeviceResources->UpdateViewport();
}

void URenderer::RenderLevel()
{ 
	//
	// 여기에 카메라 VP 업데이트 한 번 싹
	//
	if (!ULevelManager::GetInstance().GetCurrentLevel()) { return; }

	// Check show flags for primitive components
	if (IsShowFlagEnabled(EEngineShowFlags::SF_Primitives) == false) { return; }

	FPipelineDescKey PipelineDescKey;
	PipelineDescKey.BlendType = EBlendType::Opaque;
	PipelineDescKey.DepthStencilType = EDepthStencilType::Opaque;
	PipelineDescKey.ShaderType = EShaderType::StaticMeshShader;
	PipelineDescKey.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FRasterizerKey RasterizerKey;
	RasterizerKey.CullMode = D3D11_CULL_BACK;
	RasterizerKey.FillMode = D3D11_FILL_SOLID;

	if (CurrentViewMode == EViewModeIndex::Wireframe)
	{
		RasterizerKey.CullMode = D3D11_CULL_NONE;
		RasterizerKey.FillMode = D3D11_FILL_WIREFRAME;
	}
	PipelineDescKey.RasterizerKey = RasterizerKey;
	Pipeline->UpdatePipeline(Pipeline->GetOrCreatePipelineState(PipelineDescKey));
	//같은 Key(같은 매쉬)를 가진 instance의 model Matrix와 Color값을 얻어내기 위한 TMap
	//이후 PrimitiveInstanceBuffer에(GPU 버퍼를 저장) 업데이트해줄 예정
	TMap<UStaticMeshComponent*, TArray<FInstanceGPUData>> InstanceBatches;



	//Primitive를 순회하며 Key를 구하고 같은 key를 가진 component의 matrix와 color값들을
	//한데 묶음. Key(매쉬)별로 필요한 matirx와 color가 모두 저장됨s
	const TArray<UStaticMeshComponent*>& StaticMeshComponentsToRender =
		ULevelManager::GetInstance().GetCurrentLevel()->GetStaticMeshComponentsToRender();
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponentsToRender)
	{
		if (!StaticMeshComponent) { continue; }

		/////////////////////////////////////////////////////////////////최적화 방안//////////////////////////////////////////////////////////
		//다중 머티리얼을 적용하려면 Component를 가지고 렌더링을 해야하는데 Component를 batch하면 중복이 생기지 않아서 batch의 의미가 하나도 없어짐
		//UStaticMesh로 배치하고 Component를 식별할 수 있게 된다고 해도 섹션마다 다른 텍스처를 렌더링 해야하므로 결국 서브매쉬를 배치해야 할 것 같음.
		/////////////////////////////////////////////////////////////////최적화 방안//////////////////////////////////////////////////////////
		InstanceBatches[StaticMeshComponent].Emplace(StaticMeshComponent->GetWorldTransformMatrix(), StaticMeshComponent->GetColor());
	}

	//저장해놓은 인스턴스 데이터들을 순회하면서 버퍼를 업데이트하고 렌더링함.
	for (auto& Batch : InstanceBatches)
	{
		UStaticMeshComponent* StaticMeshComponent = Batch.first;
		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
		TArray<FInstanceGPUData>& Instances = Batch.second;

		if (Instances.IsEmpty())
		{
			continue;
		}

		//키로 리소스 얻어옴(GPU 버퍼, 리소스 뷰)
		FStructuredBufferResource& Resource = GetOrCreateStructuredBuffer(StaticMesh);
		//리소스가 없거나 크기가 작으면 새로 만들거나 확장함
		EnsureStructuredBufferCapacity(Resource, static_cast<uint32>(Instances.Num()));
		if (!Resource.Buffer || !Resource.ShaderResourceView)
		{
			continue;
		}

		//리소스에 인스턴스 데이터 업데이트함
		UploadStructuredBufferData(Resource, Instances.data(), static_cast<uint32>(Instances.Num()));

		Pipeline->SetVertexBuffer(StaticMesh->GetVertexBuffer(), StrideStaticMesh);
		Pipeline->SetIndexBuffer(StaticMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT);
		Pipeline->SetShaderResourceView(0, true, Resource.ShaderResourceView);


		////////////// 텍스쳐 적용 테스트 코드////////////////
		UResourceManager& ResourceManager = UResourceManager::GetInstance();
		FStaticMesh* Asset = StaticMesh->GetStaticMeshAsset();

		if (Asset && !Asset->Sections.empty())
		{
			for (int Index = 0; Index < Asset->Sections.Num(); Index++)
			{
				ID3D11ShaderResourceView* TextureSRV = nullptr;


				const UMaterial* Material = StaticMeshComponent->GetMaterial(Index);
				if (Material)
				{
					if (!Material->GetKdTextureFilePath().empty())
					{

						TextureSRV = ResourceManager.GetTexture(Material->GetKdTextureFilePath());
						if (TextureSRV)
						{
							Pipeline->SetShaderResourceView(1, false, TextureSRV);
						}
					}
				}

				if (!TextureSRV)
				{
					//매터리얼에 텍스쳐가 없거나 텍스처를 로드하지 못한 경우 흰색 텍스쳐 써서 기본 vertex 컬러 출력되도록 함.
					TextureSRV = ResourceManager.GetTexture("Data/None.dds");
					if (TextureSRV)
					{
						Pipeline->SetShaderResourceView(1, false, TextureSRV);
					}
				}
				Pipeline->DrawIndexedInstanced(Asset->Sections[Index].IndexCount, Instances.Num(), Asset->Sections[Index].IndexStart, 0, 0);
			}
		}

	}

	//아래 코드는 헤더파일 참고
	//UpdateInstanceDrawConstants(false, 0, 0);
	ID3D11ShaderResourceView* NullSRV = nullptr;
	GetDeviceContext()->VSSetShaderResources(0, 1, &NullSRV);
}

void URenderer::RenderText(const FVector& CameraLocation)
{
	if (IsShowFlagEnabled(EEngineShowFlags::SF_BillboardText) == false) { return; }

	//shader, rasterizaer state, depth stencil state, input layout 설정///////////////////
	FPipelineDescKey PipelineDescKey;
	PipelineDescKey.BlendType = EBlendType::Transparent;
	PipelineDescKey.DepthStencilType = EDepthStencilType::Transparent;
	PipelineDescKey.ShaderType = EShaderType::TextShader;
	PipelineDescKey.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FRasterizerKey RasterizerKey;
	RasterizerKey.CullMode = D3D11_CULL_NONE;
	RasterizerKey.FillMode = D3D11_FILL_SOLID;
	if (CurrentViewMode == EViewModeIndex::Wireframe)
	{
		RasterizerKey.CullMode = D3D11_CULL_NONE;
		RasterizerKey.FillMode = D3D11_FILL_WIREFRAME;
	}

	PipelineDescKey.RasterizerKey = RasterizerKey;

	Pipeline->UpdatePipeline(Pipeline->GetOrCreatePipelineState(PipelineDescKey));
	/////////////////////////////////////////////////////////////////////////////////////

	/////////////텍스처, 샘플러 설정////////////////////
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	ID3D11ShaderResourceView* Srv = ResourceManager.GetTexture("Asset/Font/Pretendard-Regular.dds");
	ID3D11SamplerState* SamplerState = ResourceManager.GetSamplerState(ESamplerType::Text);

	Pipeline->SetShaderResourceView(0, false, Srv);
	Pipeline->SetSamplerState(0, false, SamplerState);
	////////////텍스처, 샘플러 설정//////////////////////




	//text(외 투명한 물체)들은 블랜딩을 적용하기 위해서 zbuffer에 쓰기를 하지 않음, 그래서 뒤에 있는 물체가 앞에 있는 물체 위에 렌더링되는 현상이 벌어짐
	//그래서 zbuffer에 쓰지 않으면서 추가로 카메라로부터 거리순으로 정렬을 해서 멀리 있는 물체부터 그려줘야함.
	///////////////////////////////////Sorting//////////////////////////////////////////
	struct RenderObject
	{
		UTextComponent* Component;
		float DistanceToCamera;

		bool operator<(const RenderObject& Other) const
		{
			return this->DistanceToCamera > Other.DistanceToCamera;
		}
	};

	TArray<RenderObject> RenderList;

	for (UTextComponent* Component : ULevelManager::GetInstance().GetCurrentLevel()->GetTextComponentsToRender())
	{
		RenderObject Object;
		Object.Component = Component;
		Object.DistanceToCamera = (CameraLocation - Component->GetWorldLocation()).Length();
		RenderList.push_back(Object);
	}
	std::sort(RenderList.begin(), RenderList.end());
	/////////////////////////////////////////////////////////////////////////////////////



	/////////////////////////////////정렬된 리스트 순회하면서 렌더링//////////////////////
	for (RenderObject& Object : RenderList)
	{
		Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);

		USceneComponent* RootComponent = Object.Component->GetOwner()->GetRootComponent();

		//루트컴포넌트가 PrimitiveComponent가 아닌 다른 component가 될 수 있는지는 모르겠지만 일단 예외처리를 함.
		//AABB의 높이값을 이용해서 항상 엑터 위에 텍스트가 출력되도록 함
		if (RootComponent->IsA(UPrimitiveComponent::StaticClass()))
		{
			FAABB AABB = static_cast<UPrimitiveComponent*>(RootComponent)->GetWorldBounds();
			FVector Position = AABB.GetCenter();
			Position.Z = AABB.Max.Z + 1.f;
			UpdateConstant(Position, FVector(0, 0, 0), FVector(0, 0, 0));
		}
		//루트컴포넌트가 primitive가 아니면 그냥 textcomponent의 월드좌표에 z축으로 2 더해서 출력해줌
		else
		{
			UpdateConstant(Object.Component->GetWorldLocation() + FVector(0, 0, 2.0f), FVector(), FVector());
		}

		Pipeline->SetVertexBuffer(Object.Component->GetVertexBuffer(), StrideTextVertex);
		Pipeline->SetInstanceBuffer(TextInstanceBuffer, StrideTextInstance);

		//인스턴스 버퍼 업데이트(텍스트마다 다름)
		TArray<FTextInstance>* InstanceData = Object.Component->GetInstanceData();
		UpdateInstance(InstanceData);

		Pipeline->DrawInstanced(Object.Component->GetVertexNum(), InstanceData->size(), 0, 0);
	}
	////////////////////////////////////////////////////////////////////////////////////

}

void URenderer::RenderEditorPrimitive(FEditorPrimitive& Primitive, const FPipelineDescKey PipelineDescKey)
{
	Pipeline->UpdatePipeline(Pipeline->GetOrCreatePipelineState(PipelineDescKey));

	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
	UpdateConstant(Primitive.Location, Primitive.Rotation, Primitive.Scale);

	Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
	UpdateConstant(Primitive.Color);

	Pipeline->SetVertexBuffer(Primitive.Vertexbuffer, Stride);
	Pipeline->Draw(Primitive.NumVertices, 0);
}


/**
 * @brief 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
 */
void URenderer::RenderEnd() const
{
	GetSwapChain()->Present(0, 0); // 1: VSync 활성화
}

void URenderer::DrawSplitterOverlay() const
{
	if (CurrentLayout != EViewportLayout::Quad) return;
	DXGI_SWAP_CHAIN_DESC scd = {};
	GetSwapChain()->GetDesc(&scd);
	float W = (float)scd.BufferDesc.Width;
	float H = (float)scd.BufferDesc.Height;
	float x = std::clamp(VerticalRatio, 0.1f, 0.9f) * W;
	float y = std::clamp(HorizontalRatio, 0.1f, 0.9f) * H;
	ImU32 col = IM_COL32(220, 220, 220, 200);
	float thick = 2.0f;
	ImDrawList* dl = ImGui::GetForegroundDrawList();
	dl->AddLine(ImVec2(x, 0.0f), ImVec2(x, H), col, thick);
	dl->AddLine(ImVec2(0.0f, y), ImVec2(W, y), col, thick);
}

void URenderer::CreateInstanceBuffer()
{
	uint32 InByteWidth = sizeof(FTextInstance) * 100;
	D3D11_BUFFER_DESC InstanceBufferDesc = {};
	InstanceBufferDesc.ByteWidth = InByteWidth;
	InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;


	GetDevice()->CreateBuffer(&InstanceBufferDesc, nullptr, &TextInstanceBuffer);
}

void URenderer::ReleaseInstanceBuffer()
{
	if (TextInstanceBuffer)
	{
		TextInstanceBuffer->Release();
		TextInstanceBuffer = nullptr;
	}

	ReleasePrimitiveInstanceBuffers();
}

void URenderer::OnResize(uint32 InWidth, uint32 InHeight)
{
	if (!DeviceResources || !GetDevice() || !GetDeviceContext() || !GetSwapChain()) return;

	DeviceResources->ReleaseFrameBuffer();
	DeviceResources->ReleaseDepthBuffer();
	GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);

	// ResizeBuffers 호출
	HRESULT hr = GetSwapChain()->ResizeBuffers(2, InWidth, InHeight, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
	{
		UE_LOG("OnResize Failed");
		return;
	}
	DeviceResources->UpdateViewport();

	DeviceResources->CreateFrameBuffer();
	DeviceResources->CreateDepthBuffer();

	ID3D11RenderTargetView* RenderTargetView = DeviceResources->GetRenderTargetView();
	ID3D11RenderTargetView* RenderTargetViews[] = { RenderTargetView }; // 배열 생성
	GetDeviceContext()->OMSetRenderTargets(1, RenderTargetViews, DeviceResources->GetDepthStencilView());
}

/**
 * @brief Vertex Buffer 소멸 함수
 * @param InVertexBuffer
 */
void URenderer::ReleaseVertexBuffer(ID3D11Buffer* InVertexBuffer)
{
	if (InVertexBuffer) { InVertexBuffer->Release(); }
}

void URenderer::LoadViewportLayout()
{
	const path PrimaryIni   = UPathManager::GetInstance().GetRootPath()   / "Editor.ini";     // 실행 파일 폴더
	const path SecondaryIni = UPathManager::GetInstance().GetConfigPath() / "editor.ini";     // 과거 호환(Asset/Config)

	// One-time migration: if only the legacy file exists, move it to the new canonical path and remove legacy
	auto FileExists = [](const path& P) -> bool
	{
		DWORD attrs = GetFileAttributesW(P.wstring().c_str());
		return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
	};
	if (!FileExists(PrimaryIni) && FileExists(SecondaryIni))
	{
		CopyFileW(SecondaryIni.wstring().c_str(), PrimaryIni.wstring().c_str(), FALSE);
		DeleteFileW(SecondaryIni.wstring().c_str());
	}

	char Buffer[64] = {};
	auto ReadKey = [&](const char* Key, char* OutBuf, DWORD OutSize) -> bool
	{
		OutBuf[0] = '\0';
		GetPrivateProfileStringA("Viewport", Key, "", OutBuf, OutSize, PrimaryIni.string().c_str());
		return OutBuf[0] != '\0';
	};
	// Layout mode (Single/Quad)
	if (ReadKey("Layout", Buffer, sizeof(Buffer)))
	{
		std::string L = Buffer;
		if (L == "Quad")      SetViewportLayout(EViewportLayout::Quad);
		else if (L == "Single") SetViewportLayout(EViewportLayout::Single);
	}
	// Split ratios
	if (ReadKey("VerticalRatio", Buffer, sizeof(Buffer)))
	{
		float R = static_cast<float>(atof(Buffer));
		SetVerticalRatio(std::clamp(R, 0.1f, 0.9f));
	}
	if (ReadKey("HorizontalRatio", Buffer, sizeof(Buffer)))
	{
		float R = static_cast<float>(atof(Buffer));
		SetHorizontalRatio(std::clamp(R, 0.1f, 0.9f));
	}
	// Viewport type mapping
	char TypeBuf[32] = {};
	auto ReadType = [&](const char* Key, EViewportType DefaultType)
	{
		if (!ReadKey(Key, TypeBuf, sizeof(TypeBuf)))
		{
			return DefaultType;
		}
		std::string S = TypeBuf;
		if (S == "Perspective") return EViewportType::Perspective;
		if (S == "Top")         return EViewportType::Top;
		if (S == "Right")       return EViewportType::Right;
		if (S == "Front")       return EViewportType::Front;
		return DefaultType;
	};
	// Rect 수집 순서: 0=TopLeft, 1=BottomLeft, 2=TopRight, 3=BottomRight
	// 기본 매핑: TL=Perspective, TR=Top, BL=Right, BR=Front
	ViewTypes[0] = ReadType("TopLeft", EViewportType::Perspective);
	ViewTypes[1] = ReadType("BottomLeft", EViewportType::Right);
	ViewTypes[2] = ReadType("TopRight", EViewportType::Top);
	ViewTypes[3] = ReadType("BottomRight", EViewportType::Front);
}

int URenderer::GetHoveredViewportIndex(float MouseX, float MouseY, FRect& OutRect)
{
	DXGI_SWAP_CHAIN_DESC scd = {};
	GetSwapChain()->GetDesc(&scd);

	if (CurrentLayout == EViewportLayout::Single)
	{
		FRect R{0.0f, 0.0f, (float)scd.BufferDesc.Width, (float)scd.BufferDesc.Height};
		OutRect = R;
		if (MouseX >= R.X && MouseX < (R.X + R.W) && MouseY >= R.Y && MouseY < (R.Y + R.H))
		{
			return 0;
		}
		return -1;
	}
	// Quad: compute rects from ratios
	FRect Rects[4] = {};
	float W = (float)scd.BufferDesc.Width;
	float H = (float)scd.BufferDesc.Height;
	float splitX = std::clamp(VerticalRatio, 0.1f, 0.9f) * W;
	float splitY = std::clamp(HorizontalRatio, 0.1f, 0.9f) * H;
	Rects[0] = {0.0f,   0.0f,   splitX,       splitY};
	Rects[1] = {0.0f,   splitY, splitX,       H - splitY};
	Rects[2] = {splitX, 0.0f,   W - splitX,   splitY};
	Rects[3] = {splitX, splitY, W - splitX,   H - splitY};

	for (int i = 0; i < 4; ++i)
	{
		const FRect& R = Rects[i];
		if (MouseX >= R.X && MouseX < (R.X + R.W) && MouseY >= R.Y && MouseY < (R.Y + R.H))
		{
			OutRect = R;
			return i;
		}
	}
	return -1;
}

void URenderer::UpdateSplitDrag()
{
	const UInputManager& Input = UInputManager::GetInstance();
	DXGI_SWAP_CHAIN_DESC scd = {};
	GetSwapChain()->GetDesc(&scd);
	float W = (float)scd.BufferDesc.Width;
	float H = (float)scd.BufferDesc.Height;
	FPoint MP{ Input.GetMousePosition().X, Input.GetMousePosition().Y };
	bool LPressed = Input.IsKeyPressed(EKeyInput::MouseLeft);
	bool LDown    = Input.IsKeyDown(EKeyInput::MouseLeft);
	bool LRel     = Input.IsKeyReleased(EKeyInput::MouseLeft);
	static bool PrevDownSplit = false; // local edge-detect state
	bool JustPressed = LDown && !PrevDownSplit; // edge detect independent of InputManager::Update order

	float splitX = std::clamp(VerticalRatio, 0.1f, 0.9f) * W;
	float splitY = std::clamp(HorizontalRatio, 0.1f, 0.9f) * H;
	float half = SplitHotThickness * 0.5f;
	bool wasDragging = (bDragVertical || bDragHorizontal);
	if (!bDragVertical && !bDragHorizontal)
	{
		if (LPressed || JustPressed)
		{
			bool nearV = std::abs(MP.X - splitX) <= half;
			bool nearH = std::abs(MP.Y - splitY) <= half;
			// If pressing near the intersection, capture both axes simultaneously
			if (nearV && nearH) { bDragVertical = true; bDragHorizontal = true; }
			else if (nearV) { bDragVertical = true; bDragHorizontal = false; }
			else if (nearH) { bDragHorizontal = true; bDragVertical = false; }
		}
	}
	else
	{
		if (LDown)
		{
			
			// While dragging, keep the selected axis captured and update continuously
			if (bDragVertical)
			{
				VerticalRatio = std::clamp(MP.X / std::max(1.0f, W), 0.1f, 0.9f);
			}
			if (bDragHorizontal)
			{
				HorizontalRatio = std::clamp(MP.Y / std::max(1.0f, H), 0.1f, 0.9f);
			}
		}
		if (!LDown)
		{
			bDragVertical = false;
			bDragHorizontal = false;
		}
	}

	// If a drag just ended, persist the new ratios
	if (wasDragging && !LDown)
	{
		SaveViewportLayout();
	}

	// update prev
	PrevDownSplit = LDown;
}

void URenderer::SaveViewportLayout() const
{
	// 실행 파일 폴더의 Editor.ini에 저장
	const path ConfigFilePath = UPathManager::GetInstance().GetRootPath() / "Editor.ini";
	// Layout mode
	WritePrivateProfileStringA("Viewport", "Layout", (GetViewportLayout() == EViewportLayout::Quad) ? "Quad" : "Single", ConfigFilePath.string().c_str());
	// Split ratios
	char Buf[64];
	snprintf(Buf, sizeof(Buf), "%.6f", std::clamp(VerticalRatio, 0.1f, 0.9f));
	WritePrivateProfileStringA("Viewport", "VerticalRatio", Buf, ConfigFilePath.string().c_str());
	snprintf(Buf, sizeof(Buf), "%.6f", std::clamp(HorizontalRatio, 0.1f, 0.9f));
	WritePrivateProfileStringA("Viewport", "HorizontalRatio", Buf, ConfigFilePath.string().c_str());
	// View types
	auto WriteType = [&](const char* Key, EViewportType T)
	{
		const char* V = "Perspective";
		switch (T)
		{
		case EViewportType::Top: V = "Top"; break;
		case EViewportType::Right: V = "Right"; break;
		case EViewportType::Front: V = "Front"; break;
		default: V = "Perspective"; break;
		}
		WritePrivateProfileStringA("Viewport", Key, V, ConfigFilePath.string().c_str());
	};
	WriteType("TopLeft", ViewTypes[0]);
	WriteType("BottomLeft", ViewTypes[1]);
	WriteType("TopRight", ViewTypes[2]);
	WriteType("BottomRight", ViewTypes[3]);
}

/**
 * @brief 상수 버퍼 생성 함수
 */
void URenderer::CreateConstantBuffer()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	/**
	 * @brief 모델에 사용될 상수 버퍼 생성
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FMatrix) + 0xf & 0xfffffff0;
		// ensure constant buffer size is multiple of 16 bytes
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferModels);
	}

	/**
	 * @brief 색상 수정에 사용할 상수 버퍼
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FVector4) + 0xf & 0xfffffff0;
		// ensure constant buffer size is multiple of 16 bytes
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferColor);
	}

	/**
	 * @brief 카메라에 사용될 상수 버퍼 생성
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FViewProjConstants) + 0xf & 0xfffffff0;
		// ensure constant buffer size is multiple of 16 bytes
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferPerFrame);
	}

	/**
	 * @brief 폰트에 사용될 조회 테이블 상수 버퍼 생성
	 */
	{
		const TArray<FCharacterInfo>& CharTable = ResourceManager.GetCharInfos();
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FCharacterInfo) * CharTable.Num();
		ConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ConstantBufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA CharTableData = {};
		CharTableData.pSysMem = CharTable.data();
		GetDevice()->CreateBuffer(&ConstantBufferDesc, &CharTableData, &ConstantBufferCharTable);

		Pipeline->SetConstantBuffer(4, true, ConstantBufferCharTable);
	}

	//UpdateInstanceDrawConstants(false, 0, 0);
}

/**
 * @brief 상수 버퍼 소멸 함수
 */
void URenderer::ReleaseConstantBuffer()
{
	if (ConstantBufferModels)
	{
		ConstantBufferModels->Release();
		ConstantBufferModels = nullptr;
	}

	if (ConstantBufferColor)
	{
		ConstantBufferColor->Release();
		ConstantBufferColor = nullptr;
	}

	if (ConstantBufferPerFrame)
	{
		ConstantBufferPerFrame->Release();
		ConstantBufferPerFrame = nullptr;
	}

	if (ConstantBufferCharTable)
	{
		ConstantBufferCharTable->Release();
		ConstantBufferCharTable = nullptr;
	}
}

void URenderer::UpdateConstant(const FMatrix& InMatrix) const
{
	if (ConstantBufferModels)
	{
		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;
		GetDeviceContext()->Map(ConstantBufferModels, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		FMatrix* Constants = static_cast<FMatrix*>(ConstantBufferMSR.pData);
		*Constants = InMatrix;
		GetDeviceContext()->Unmap(ConstantBufferModels, 0);
	}
}

/**
 * @brief 상수 버퍼 업데이트 함수
 * @param InOffset
 * @param InScale Ball Size
 */
void URenderer::UpdateConstant(const FVector& InPosition, const FVector& InRotation, const FVector& InScale) const
{
	if (ConstantBufferModels)
	{
		D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

		GetDeviceContext()->Map(ConstantBufferModels, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
		// update constant buffer every frame
		FMatrix* constants = (FMatrix*)constantbufferMSR.pData;
		{
			*constants = FMatrix::GetModelMatrix(InPosition, FVector::GetDegreeToRadian(InRotation), InScale);
		}
		GetDeviceContext()->Unmap(ConstantBufferModels, 0);
	}
}

void URenderer::UpdateConstant(const FViewProjConstants& InViewProjConstants) const
{
	Pipeline->SetConstantBuffer(1, false, ConstantBufferPerFrame);
	Pipeline->SetConstantBuffer(1, true, ConstantBufferPerFrame);

	if (ConstantBufferPerFrame)
	{
		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR = {};

		GetDeviceContext()->Map(ConstantBufferPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		// update constant buffer every frame
		FViewProjConstants* ViewProjectionConstants = (FViewProjConstants*)ConstantBufferMSR.pData;
		{
			ViewProjectionConstants->View = InViewProjConstants.View;
			ViewProjectionConstants->Projection = InViewProjConstants.Projection;
			ViewProjectionConstants->ViewModeIndex = static_cast<uint32>(CurrentViewMode);
		}
		GetDeviceContext()->Unmap(ConstantBufferPerFrame, 0);
	}
}

void URenderer::UpdateConstant(const FVector4& Color) const
{
	Pipeline->SetConstantBuffer(2, false, ConstantBufferColor);

	if (ConstantBufferColor)
	{
		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR = {};

		GetDeviceContext()->Map(ConstantBufferColor, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		// update constant buffer every frame
		FVector4* ColorConstants = (FVector4*)ConstantBufferMSR.pData;
		{
			ColorConstants->X = Color.X;
			ColorConstants->Y = Color.Y;
			ColorConstants->Z = Color.Z;
			ColorConstants->W = Color.W;
		}
		GetDeviceContext()->Unmap(ConstantBufferColor, 0);
	}
}

void URenderer::UpdateInstance(const TArray<FTextInstance>* Instance)
{
	if (TextInstanceBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE InstanceBufferMSR = {};

		GetDeviceContext()->Map(TextInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &InstanceBufferMSR);
		// update constant buffer every frame
		memcpy(InstanceBufferMSR.pData, Instance->data(), sizeof(FTextInstance) * Instance->size());

		GetDeviceContext()->Unmap(TextInstanceBuffer, 0);
	}
}

URenderer::FStructuredBufferResource& URenderer::GetOrCreateStructuredBuffer(UStaticMesh* InKey)
{
	return StaticMeshStructuredBuffers[InKey];
}

void URenderer::EnsureStructuredBufferCapacity(FStructuredBufferResource& InResource, uint32 InRequiredInstanceCount)
{
	if (InRequiredInstanceCount == 0)
	{
		return;
	}

	if (InResource.Capacity >= InRequiredInstanceCount && InResource.Buffer && InResource.ShaderResourceView)
	{
		return;
	}

	uint32 NewCapacity = InResource.Capacity > 0 ? InResource.Capacity : 64u;
	while (NewCapacity < InRequiredInstanceCount)
	{
		NewCapacity *= 2u;
	}

	if (InResource.ShaderResourceView)
	{
		InResource.ShaderResourceView->Release();
		InResource.ShaderResourceView = nullptr;
	}

	if (InResource.Buffer)
	{
		InResource.Buffer->Release();
		InResource.Buffer = nullptr;
	}

	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(FInstanceGPUData) * NewCapacity;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(FInstanceGPUData);

	ID3D11Device* Device = GetDevice();
	HRESULT Hr = Device->CreateBuffer(&BufferDesc, nullptr, &InResource.Buffer);
	if (FAILED(Hr) || !InResource.Buffer)
	{
		UE_LOG("Failed to create instance structured buffer");
		InResource.Capacity = 0;
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SrvDesc.Buffer.FirstElement = 0;
	SrvDesc.Buffer.NumElements = NewCapacity;

	Hr = Device->CreateShaderResourceView(InResource.Buffer, &SrvDesc, &InResource.ShaderResourceView);
	if (FAILED(Hr) || !InResource.ShaderResourceView)
	{
		UE_LOG("Failed to create instance buffer SRV");
		InResource.Buffer->Release();
		InResource.Buffer = nullptr;
		InResource.Capacity = 0;
		return;
	}

	InResource.Capacity = NewCapacity;
}

void URenderer::UploadStructuredBufferData(FStructuredBufferResource& InResource, const void* InData,
	uint32 InInstanceCount)
{
	if (!InResource.Buffer || InInstanceCount == 0)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE Mapped = {};
	HRESULT Hr = GetDeviceContext()->Map(InResource.Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
	if (FAILED(Hr))
	{
		UE_LOG("Failed to map instance buffer");
		return;
	}

	const size_t CopySize = sizeof(FInstanceGPUData) * static_cast<size_t>(InInstanceCount);
	memcpy(Mapped.pData, InData, CopySize);
	GetDeviceContext()->Unmap(InResource.Buffer, 0);
}

void URenderer::ReleasePrimitiveInstanceBuffers()
{
	for (auto& Pair : StaticMeshStructuredBuffers)
	{
		if (Pair.second.ShaderResourceView)
		{
			Pair.second.ShaderResourceView->Release();
			Pair.second.ShaderResourceView = nullptr;
		}

		if (Pair.second.Buffer)
		{
			Pair.second.Buffer->Release();
			Pair.second.Buffer = nullptr;
		}

		Pair.second.Capacity = 0;
	}

	StaticMeshStructuredBuffers.Empty();
}
