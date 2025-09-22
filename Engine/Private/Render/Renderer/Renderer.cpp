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
}

void URenderer::Release()
{
	/** LineBatchRenderer 해제 */
	ULineBatchRenderer::GetInstance().Release();

	ReleaseConstantBuffer();

	ReleaseResource();
	ReleaseInstanceBuffer();


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



void URenderer::Update(UEditor* Editor)
{
	RenderBegin();

	RenderLevel();
	// Editor->RenderEditor();
	Editor->RenderEditorBatched();
	RenderText(Editor->GetCameraLocation());
	//RenderLines();

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

	ID3D11RenderTargetView* RenderTargetViews[] = {RenderTargetView}; // 배열 생성

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
	TMap<UStaticMesh*, TArray<FInstanceGPUData>> InstanceBatches;



	//Primitive를 순회하며 Key를 구하고 같은 key를 가진 component의 matrix와 color값들을
	//한데 묶음. Key(매쉬)별로 필요한 matirx와 color가 모두 저장됨s
	const TArray<UStaticMeshComponent*>& StaticMeshComponentsToRender =
		ULevelManager::GetInstance().GetCurrentLevel()->GetStaticMeshComponentsToRender();
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponentsToRender)
	{
		if (!StaticMeshComponent) { continue; }

		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
		if (!StaticMesh)
		{
			continue;
		}

		InstanceBatches[StaticMesh].Emplace(StaticMeshComponent->GetWorldTransformMatrix(), StaticMeshComponent->GetColor());
	}

	//저장해놓은 인스턴스 데이터들을 순회하면서 버퍼를 업데이트하고 렌더링함.
	for (auto& Batch : InstanceBatches)
	{
		UStaticMesh* StaticMesh = Batch.first;
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
				const FString& MaterialName = Asset->Sections[Index].MaterialName;
				if (!MaterialName.empty())
				{
					const FString& TexturePath = StaticMesh->GetKdTextureFilePath(MaterialName);
					if (!TexturePath.empty())
					{
						TextureSRV = ResourceManager.GetTexture(TexturePath);
						if (TextureSRV)
						{
							Pipeline->SetShaderResourceView(1, false, TextureSRV);
						}   
					}
				}
				if (!TextureSRV)
				{
					//에셋에 텍스쳐 없는 경우 흰색 텍스쳐 써서 기본 vertex 컬러 출력되도록 함.
					TextureSRV = ResourceManager.GetTexture("Data/whitespace.dds");
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
	ID3D11RenderTargetView* RenderTargetViews[] = {RenderTargetView}; // 배열 생성
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
