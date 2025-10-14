#include "pch.h"

#include "BillboardComponent.h"
#include "TextRenderComponent.h"
#include "Shader.h"
#include "StaticMesh.h"
#include "TextQuad.h"
#include "StaticMeshComponent.h"
#include "RenderingStats.h"
#include "UI/StatsOverlayD2D.h"


URenderer::URenderer(URHIDevice* InDevice) : RHIDevice(InDevice)
{
    InitializeLineBatch();
}

URenderer::~URenderer()
{
    if (LineBatchData)
    {
        delete LineBatchData;
    }
}

void URenderer::BeginFrame()
{
    // 렌더링 통계 수집 시작
    URenderingStatsCollector::GetInstance().BeginFrame();
    
    // 상태 추적 리셋
    ResetRenderStateTracking();
    
    // 백버퍼/깊이버퍼를 클리어
    RHIDevice->ClearBackBuffer();  // 배경색
    RHIDevice->ClearDepthBuffer(1.0f, 0);                 // 깊이값 초기화
    RHIDevice->CreateBlendState();
    RHIDevice->IASetPrimitiveTopology();
    // RS
    RHIDevice->RSSetViewport();

    //OM
    //RHIDevice->OMSetBlendState();
    RHIDevice->OMSetRenderTargets();
}

void URenderer::PrepareShader(FShader& InShader)
{
    RHIDevice->GetDeviceContext()->VSSetShader(InShader.SimpleVertexShader, nullptr, 0);
    RHIDevice->GetDeviceContext()->PSSetShader(InShader.SimplePixelShader, nullptr, 0);
    RHIDevice->GetDeviceContext()->IASetInputLayout(InShader.SimpleInputLayout);
}

void URenderer::PrepareShader(UShader* InShader)
{
    // 셰이더 변경 추적
    if (LastShader != InShader)
    {
        URenderingStatsCollector::GetInstance().IncrementShaderChanges();
        LastShader = InShader;
    }
    
    RHIDevice->GetDeviceContext()->VSSetShader(InShader->GetVertexShader(), nullptr, 0);
    RHIDevice->GetDeviceContext()->PSSetShader(InShader->GetPixelShader(), nullptr, 0);
    RHIDevice->GetDeviceContext()->IASetInputLayout(InShader->GetInputLayout());
}

void URenderer::OMSetBlendState(bool bIsChecked)
{
    if (bIsChecked == true)
    {
        RHIDevice->OMSetBlendState(true);
    }
    else
    {
        RHIDevice->OMSetBlendState(false);
    }
}

void URenderer::OMSetBlendState(EBlendMode BlendMode)
{
    RHIDevice->OMSetBlendState(BlendMode);
}

void URenderer::RSSetState(EViewModeIndex ViewModeIndex)
{
    RHIDevice->RSSetState(ViewModeIndex);
}

void URenderer::RSSetFrontCullState()
{
    RHIDevice->RSSetFrontCullState();
}

void URenderer::RSSetNoCullState()
{
    RHIDevice->RSSetNoCullState();
}

void URenderer::RSSetDefaultState()
{
    RHIDevice->RSSetDefaultState();
}

void URenderer::RSSetDecalState()
{
    RHIDevice->RSSetDecalState();
}

void URenderer::UpdateConstantBuffer(const FMatrix& ModelMatrix, const FMatrix& ViewMatrix, const FMatrix& ProjMatrix)
{
    RHIDevice->UpdateConstantBuffers(ModelMatrix, ViewMatrix, ProjMatrix);
}

void URenderer::UpdateHighLightConstantBuffer(const uint32 InPicked, const FVector& InColor, const uint32 X, const uint32 Y, const uint32 Z, const uint32 Gizmo)
{
    RHIDevice->UpdateHighLightConstantBuffers(InPicked, InColor, X, Y, Z, Gizmo);
}

void URenderer::UpdateBillboardConstantBuffers(const FVector& pos,const FMatrix& ViewMatrix, const FMatrix& ProjMatrix, const FVector& CameraRight, const FVector& CameraUp)
{
    RHIDevice->UpdateBillboardConstantBuffers(pos,ViewMatrix, ProjMatrix, CameraRight, CameraUp);
}

void URenderer::UpdatePixelConstantBuffers(const FObjMaterialInfo& InMaterialInfo, bool bHasMaterial, bool bHasTexture)
{
    RHIDevice->UpdatePixelConstantBuffers(InMaterialInfo, bHasMaterial, bHasTexture);
}

void URenderer::UpdateColorBuffer(const FVector4& Color)
{
    RHIDevice->UpdateColorConstantBuffers(Color);
}

void URenderer::UpdateInvWorldBuffer(const FMatrix& InvWorldMatrix, const FMatrix& InvViewProjMatrix)
{
    RHIDevice->UpdateInvWorldConstantBuffer(InvWorldMatrix, InvViewProjMatrix);
}

void URenderer::UpdateInvMatrixBuffer(const FMatrix& InvWorldMatrix, const FMatrix& InvViewMatrix, const FMatrix& InvProjMatrix)
{
    RHIDevice->UpdateInvMatrixConstantBuffer(InvWorldMatrix, InvViewMatrix, InvProjMatrix);
}

void URenderer::UpdateViewportBuffer(float StartX, float StartY, float SizeX, float SizeY)
{
    static_cast<D3D11RHI*>(RHIDevice)->UpdateViewportConstantBuffer(StartX, StartY, SizeX, SizeY);
}

void URenderer::UpdateUVScroll(const FVector2D& Speed, float TimeSec)
{
    RHIDevice->UpdateUVScrollConstantBuffers(Speed, TimeSec);
}

void URenderer::UpdateDecalConstantBuffer(const FMatrix& InWorldMVP, const FMatrix& InDecalMVP, const float InAlpha)
{
    RHIDevice->UpdateDecalConstantBuffer(InWorldMVP, InDecalMVP, InAlpha);
}

void URenderer::UpdateHeightFogConstantBuffer(
    const FLinearColor& FogInscatteringColor,
    float FogDensity,
    float FogHeightFalloff,
    float StartDistance,
    float FogCutoffDistance,
    float FogMaxOpacity,
    float FogHeightOffset)
{
    RHIDevice->UpdateHeightFogConstantBuffer(
        FogInscatteringColor,
        FogDensity,
        FogHeightFalloff,
        StartDistance,
        FogCutoffDistance,
        FogMaxOpacity,
        FogHeightOffset
    );
}

void URenderer::UpdateSceneDepthBuffer(float Near, float Far)
{
    RHIDevice->UpdateSceneDepthBuffer(Near, Far);
}

void URenderer::UpdateFireBallConstantBuffer(const FireBallBufferType& InFireBallData)
{
    RHIDevice->UpdateFireBallConstantBuffer(InFireBallData);
}

void URenderer::ProjectDecalToStaticMesh(UDecalComponent* Comp, UStaticMesh* InMesh, D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
    RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(InTopology);

    UINT stride = 0;
    stride = sizeof(FVertexDynamic);

    UINT offset = 0;

    ID3D11Buffer* VertexBuffer = InMesh->GetVertexBuffer();
    ID3D11Buffer* IndexBuffer = InMesh->GetIndexBuffer();
    uint32 VertexCount = InMesh->GetVertexCount();
    uint32 IndexCount = InMesh->GetIndexCount();

    RHIDevice->GetDeviceContext()->IASetVertexBuffers(
        0, 1, &VertexBuffer, &stride, &offset
    );

    RHIDevice->GetDeviceContext()->IASetIndexBuffer(
        IndexBuffer, DXGI_FORMAT_R32_UINT, 0
    );

    URenderingStatsCollector& StatsCollector = URenderingStatsCollector::GetInstance();

    // 매테리얼 변경 추적
    UMaterial* CompMaterial = Comp->GetMaterial();
    if (LastMaterial != CompMaterial)
    {
        StatsCollector.IncrementMaterialChanges();
        LastMaterial = CompMaterial;
    }

    UShader* CompShader = CompMaterial->GetShader();
    // 셰이더 변경 추적
    if (LastShader != CompShader)
    {
        StatsCollector.IncrementShaderChanges();
        LastShader = CompShader;
    }

    RHIDevice->GetDeviceContext()->IASetInputLayout(CompShader->GetInputLayout());

    // 텍스처 변경 추적 (텍스처 비교)
    UTexture* CompTexture = CompMaterial->GetTexture();
    if (LastTexture != CompTexture)
    {
        StatsCollector.IncrementTextureChanges();
        LastTexture = CompTexture;
    }

    ID3D11ShaderResourceView* TextureSRV = CompTexture->GetShaderResourceView();

    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &TextureSRV);
    RHIDevice->PSSetDefaultSampler(0);

    RHIDevice->GetDeviceContext()->DrawIndexed(IndexCount, 0, 0);
    StatsCollector.IncrementDrawCalls();
    StatsCollector.IncrementBasePassDrawCalls();
}

void URenderer::DrawIndexedPrimitiveComponent(UStaticMesh* InMesh, D3D11_PRIMITIVE_TOPOLOGY InTopology, const TArray<FMaterialSlot>& InComponentMaterialSlots)
{
    URenderingStatsCollector& StatsCollector = URenderingStatsCollector::GetInstance();
    
    // 디버그: StaticMesh 렌더링 통계
    
    UINT stride = 0;
    switch (InMesh->GetVertexType())
    {
    case EVertexLayoutType::PositionColor:
        stride = sizeof(FVertexSimple);
        break;
    case EVertexLayoutType::PositionColorTexturNormal:
        stride = sizeof(FVertexDynamic);
        break;
    case EVertexLayoutType::PositionBillBoard:
        stride = sizeof(FBillboardVertexInfo_GPU);
        break;
    default:
        // Handle unknown or unsupported vertex types
        assert(false && "Unknown vertex type!");
        return; // or log an error
    }
    UINT offset = 0;

    ID3D11Buffer* VertexBuffer = InMesh->GetVertexBuffer();
    ID3D11Buffer* IndexBuffer = InMesh->GetIndexBuffer();
    uint32 VertexCount = InMesh->GetVertexCount();
    uint32 IndexCount = InMesh->GetIndexCount();

    RHIDevice->GetDeviceContext()->IASetVertexBuffers(
        0, 1, &VertexBuffer, &stride, &offset
    );

    RHIDevice->GetDeviceContext()->IASetIndexBuffer(
        IndexBuffer, DXGI_FORMAT_R32_UINT, 0
    );

    RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(InTopology);
    RHIDevice->PSSetDefaultSampler(0);

    if (InMesh->HasMaterial())
    {
        const TArray<FGroupInfo>& MeshGroupInfos = InMesh->GetMeshGroupInfo();
        const uint32 NumMeshGroupInfos = static_cast<uint32>(MeshGroupInfos.size());
        for (uint32 i = 0; i < NumMeshGroupInfos; ++i)
        {
            const UMaterial* const Material = UResourceManager::GetInstance().Get<UMaterial>(InComponentMaterialSlots[i].MaterialName);
			if (!Material) continue; // Material이 없으면 건너뜀

            const FObjMaterialInfo& MaterialInfo = Material->GetMaterialInfo();
            bool bHasTexture = !(MaterialInfo.DiffuseTextureFileName == FName::None());
            
            // 재료 변경 추적
            if (LastMaterial != Material)
            {
                StatsCollector.IncrementMaterialChanges();
                LastMaterial = const_cast<UMaterial*>(Material);
            }
            
            FTextureData* TextureData = nullptr;
            if (bHasTexture)
            {
                TextureData = UResourceManager::GetInstance().CreateOrGetTextureData(MaterialInfo.DiffuseTextureFileName);
                
                // 텍스처 변경 추적 (임시로 FTextureData*를 UTexture*로 캠스트)
                UTexture* CurrentTexture = reinterpret_cast<UTexture*>(TextureData);
                if (LastTexture != CurrentTexture)
                {
                    StatsCollector.IncrementTextureChanges();
                    LastTexture = CurrentTexture;
                }
                
                RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &(TextureData->TextureSRV));
            }
            
            RHIDevice->UpdatePixelConstantBuffers(MaterialInfo, true, bHasTexture); // PSSet도 해줌
            
            // DrawCall 수실행 및 통계 추가
            RHIDevice->GetDeviceContext()->DrawIndexed(MeshGroupInfos[i].IndexCount, MeshGroupInfos[i].StartIndex, 0);
            StatsCollector.IncrementDrawCalls();
        }
    }
    else
    {
        FObjMaterialInfo ObjMaterialInfo;
        RHIDevice->UpdatePixelConstantBuffers(ObjMaterialInfo, false, false); // PSSet도 해줌
        RHIDevice->GetDeviceContext()->DrawIndexed(IndexCount, 0, 0);
        StatsCollector.IncrementDrawCalls();
    }
}

void URenderer::DrawIndexedPrimitiveComponent(UTextRenderComponent* Comp, D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
    URenderingStatsCollector& StatsCollector = URenderingStatsCollector::GetInstance();
    
    // 디버그: TextRenderComponent 렌더링 통계
    
    UINT Stride = sizeof(FBillboardVertexInfo_GPU);
    ID3D11Buffer* VertexBuff = Comp->GetStaticMesh()->GetVertexBuffer();
    ID3D11Buffer* IndexBuff = Comp->GetStaticMesh()->GetIndexBuffer();

    // 매테리얼 변경 추적
    UMaterial* CompMaterial = Comp->GetMaterial();
    if (LastMaterial != CompMaterial)
    {
        StatsCollector.IncrementMaterialChanges();
        LastMaterial = CompMaterial;
    }
    
    UShader* CompShader = CompMaterial->GetShader();
    // 셰이더 변경 추적
    if (LastShader != CompShader)
    {
        StatsCollector.IncrementShaderChanges();
        LastShader = CompShader;
    }
    
    RHIDevice->GetDeviceContext()->IASetInputLayout(CompShader->GetInputLayout());

    
    UINT offset = 0;
    RHIDevice->GetDeviceContext()->IASetVertexBuffers(
        0, 1, &VertexBuff, &Stride, &offset
    );
    RHIDevice->GetDeviceContext()->IASetIndexBuffer(
        IndexBuff, DXGI_FORMAT_R32_UINT, 0
    );

    // 텍스처 변경 추적 (텍스처 비교)
    UTexture* CompTexture = CompMaterial->GetTexture();
    if (LastTexture != CompTexture)
    {
        StatsCollector.IncrementTextureChanges();
        LastTexture = CompTexture;
    }
    
    ID3D11ShaderResourceView* TextureSRV = CompTexture->GetShaderResourceView();
    RHIDevice->PSSetDefaultSampler(0);
    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &TextureSRV);
    RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(InTopology);
    RHIDevice->GetDeviceContext()->DrawIndexed(Comp->GetStaticMesh()->GetIndexCount(), 0, 0);
    StatsCollector.IncrementDrawCalls();
}


void URenderer::DrawIndexedPrimitiveComponent(UBillboardComponent* Comp, D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
    URenderingStatsCollector& StatsCollector = URenderingStatsCollector::GetInstance();
    
    // 디버그: TextRenderComponent 렌더링 통계
    
    UINT Stride = sizeof(FBillboardVertexInfo_GPU);
    ID3D11Buffer* VertexBuff = Comp->GetStaticMesh()->GetVertexBuffer();
    ID3D11Buffer* IndexBuff = Comp->GetStaticMesh()->GetIndexBuffer();

    // 매테리얼 변경 추적
    UMaterial* CompMaterial = Comp->GetMaterial();
    if (LastMaterial != CompMaterial)
    {
        StatsCollector.IncrementMaterialChanges();
        LastMaterial = CompMaterial;
    }
    
    UShader* CompShader = CompMaterial->GetShader();
    // 셰이더 변경 추적
    if (LastShader != CompShader)
    {
        StatsCollector.IncrementShaderChanges();
        LastShader = CompShader;
    }
    
    RHIDevice->GetDeviceContext()->IASetInputLayout(CompShader->GetInputLayout());

    
    UINT offset = 0;
    RHIDevice->GetDeviceContext()->IASetVertexBuffers(
        0, 1, &VertexBuff, &Stride, &offset
    );
    RHIDevice->GetDeviceContext()->IASetIndexBuffer(
        IndexBuff, DXGI_FORMAT_R32_UINT, 0
    );

    // 텍스처 변경 추적 (텍스처 비교)
    UTexture* CompTexture = CompMaterial->GetTexture();
    if (LastTexture != CompTexture)
    {
        StatsCollector.IncrementTextureChanges();
        LastTexture = CompTexture;
    }
    
    ID3D11ShaderResourceView* TextureSRV = CompTexture->GetShaderResourceView();
    RHIDevice->PSSetDefaultSampler(0);
    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &TextureSRV);
    RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(InTopology);
    RHIDevice->GetDeviceContext()->DrawIndexed(Comp->GetStaticMesh()->GetIndexCount(), 0, 0);
    StatsCollector.IncrementDrawCalls();
}
/**
* @brief StaticMesh를 그리는 단순화된 DrawIndexPrimitiveComponent
* 정점 및 인덱스 버퍼 바인딩
* 셰이더, 텍스쳐, 머티리얼 변경 X, 상수버퍼 업데이트 X
*/
void URenderer::DrawSimpleMesh(UStaticMesh* InMesh)
{
    if (!InMesh)
    {
        return;
    }
    UINT Stride = 0;
    switch (InMesh->GetVertexType())
    {
    case EVertexLayoutType::PositionColor:
        Stride = sizeof(FVertexSimple);
        break;
    case EVertexLayoutType::PositionColorTexturNormal:
        Stride = sizeof(FVertexDynamic);
        break;
    case EVertexLayoutType::PositionBillBoard:
        Stride = sizeof(FBillboardVertexInfo_GPU);
        break;
    default:
        // Handle unknown or unsupported vertex types
        assert(false && "Unknown vertex type!");
        return; // or log an error
    }

    UINT Offset = 0;
    ID3D11Buffer* VertexBuffer = InMesh->GetVertexBuffer();
    ID3D11Buffer* IndexBuffer = InMesh->GetIndexBuffer();

    RHIDevice->GetDeviceContext()->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    RHIDevice->GetDeviceContext()->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 서브 메시가 있는지 판단 위해 사용
    if (InMesh->HasMaterial())
    {
        const TArray<FGroupInfo> MeshGroupInfos = InMesh->GetMeshGroupInfo();
        for (const auto& GroupInfo : MeshGroupInfos)
        {
            RHIDevice->GetDeviceContext()->DrawIndexed(GroupInfo.IndexCount, GroupInfo.StartIndex, 0);
            URenderingStatsCollector::GetInstance().IncrementDrawCalls();
        }
    }
    else
    {
        // 서브 메쉬 없는 경우
        RHIDevice->GetDeviceContext()->DrawIndexed(InMesh->GetIndexCount(), 0, 0);
        URenderingStatsCollector::GetInstance().IncrementDrawCalls();
    }
}


void URenderer::ClearFireBallData()
{
    FrameFireBallData.clear();
}

void URenderer::AddFireBallToScene(const FireBallBufferType& InData)
{
    FrameFireBallData.Push(InData);
}

const TArray<FireBallBufferType>& URenderer::GetFrameFireBallData() const
{
    return FrameFireBallData;
}

void URenderer::SetViewModeType(EViewModeIndex ViewModeIndex)
{
    RHIDevice->RSSetState(ViewModeIndex);
    if(ViewModeIndex == EViewModeIndex::VMI_Wireframe)
        RHIDevice->UpdateColorConstantBuffers(FVector4{ 1.f, 0.f, 0.f, 1.f });
    else
        RHIDevice->UpdateColorConstantBuffers(FVector4{ 1.f, 1.f, 1.f, 0.f });
}

void URenderer::EndFrame()
{
    // 렌더링 통계 수집 종료
    URenderingStatsCollector& StatsCollector = URenderingStatsCollector::GetInstance();
    StatsCollector.EndFrame();
    
    // 현재 프레임 통계를 업데이트
    const FRenderingStats& CurrentStats = StatsCollector.GetCurrentFrameStats();
    StatsCollector.UpdateFrameStats(CurrentStats);
    
    // 평균 통계를 얻어서 오버레이에 업데이트
    const FRenderingStats& AvgStats = StatsCollector.GetAverageStats();
    UStatsOverlayD2D::Get().UpdateRenderingStats(
        AvgStats.TotalDrawCalls,
        AvgStats.MaterialChanges,
        AvgStats.TextureChanges,
        AvgStats.ShaderChanges
    );
    
    RHIDevice->Present();
}

void URenderer::OMSetDepthStencilState(EComparisonFunc Func)
{
    RHIDevice->OmSetDepthStencilState(Func);
}

void URenderer::InitializeLineBatch()
{
    // Create UDynamicMesh for efficient line batching
    DynamicLineMesh = UResourceManager::GetInstance().Load<ULineDynamicMesh>("Line");
    
    // Initialize with maximum capacity (MAX_LINES * 2 vertices, MAX_LINES * 2 indices)
    uint32 maxVertices = MAX_LINES * 2;
    uint32 maxIndices = MAX_LINES * 2;
    DynamicLineMesh->Load(maxVertices, maxIndices, RHIDevice->GetDevice());

    // Create FMeshData for accumulating line data
    LineBatchData = new FMeshData();
    
    // Load line shader
    LineShader = UResourceManager::GetInstance().Load<UShader>("ShaderLine.hlsl");
}

void URenderer::BeginLineBatch()
{
    if (!LineBatchData) return;
    
    bLineBatchActive = true;
    
    // Clear previous batch data
    LineBatchData->Vertices.clear();
    LineBatchData->Color.clear();
    LineBatchData->Indices.clear();
}

void URenderer::AddLine(const FVector& Start, const FVector& End, const FVector4& Color)
{
    if (!bLineBatchActive || !LineBatchData) return;
    
    uint32 startIndex = static_cast<uint32>(LineBatchData->Vertices.size());
    
    // Add vertices
    LineBatchData->Vertices.push_back(Start);
    LineBatchData->Vertices.push_back(End);
    
    // Add colors
    LineBatchData->Color.push_back(Color);
    LineBatchData->Color.push_back(Color);
    
    // Add indices for line (2 vertices per line)
    LineBatchData->Indices.push_back(startIndex);
    LineBatchData->Indices.push_back(startIndex + 1);
}

void URenderer::AddLines(const TArray<FVector>& StartPoints, const TArray<FVector>& EndPoints, const TArray<FVector4>& Colors)
{
    if (!bLineBatchActive || !LineBatchData) return;
    
    // Validate input arrays have same size
    if (StartPoints.size() != EndPoints.size() || StartPoints.size() != Colors.size())
        return;
    
    uint32 startIndex = static_cast<uint32>(LineBatchData->Vertices.size());
    
    // Reserve space for efficiency
    size_t lineCount = StartPoints.size();
    LineBatchData->Vertices.reserve(LineBatchData->Vertices.size() + lineCount * 2);
    LineBatchData->Color.reserve(LineBatchData->Color.size() + lineCount * 2);
    LineBatchData->Indices.reserve(LineBatchData->Indices.size() + lineCount * 2);
    
    // Add all lines at once
    for (size_t i = 0; i < lineCount; ++i)
    {
        uint32 currentIndex = startIndex + static_cast<uint32>(i * 2);
        
        // Add vertices
        LineBatchData->Vertices.push_back(StartPoints[i]);
        LineBatchData->Vertices.push_back(EndPoints[i]);
        
        // Add colors
        LineBatchData->Color.push_back(Colors[i]);
        LineBatchData->Color.push_back(Colors[i]);
        
        // Add indices for line (2 vertices per line)
        LineBatchData->Indices.push_back(currentIndex);
        LineBatchData->Indices.push_back(currentIndex + 1);
    }
}

void URenderer::DrawAABB(const FVector& Min, const FVector& Max, const FVector4& Color)
{
    if (!bLineBatchActive) return;

    FVector Vertices[8];
    Vertices[0] = FVector(Min.X, Min.Y, Min.Z);
    Vertices[1] = FVector(Max.X, Min.Y, Min.Z);
    Vertices[2] = FVector(Max.X, Max.Y, Min.Z);
    Vertices[3] = FVector(Min.X, Max.Y, Min.Z);
    Vertices[4] = FVector(Min.X, Min.Y, Max.Z);
    Vertices[5] = FVector(Max.X, Min.Y, Max.Z);
    Vertices[6] = FVector(Max.X, Max.Y, Max.Z);
    Vertices[7] = FVector(Min.X, Max.Y, Max.Z);

    // Bottom face
    AddLine(Vertices[0], Vertices[1], Color);
    AddLine(Vertices[1], Vertices[2], Color);
    AddLine(Vertices[2], Vertices[3], Color);
    AddLine(Vertices[3], Vertices[0], Color);

    // Top face
    AddLine(Vertices[4], Vertices[5], Color);
    AddLine(Vertices[5], Vertices[6], Color);
    AddLine(Vertices[6], Vertices[7], Color);
    AddLine(Vertices[7], Vertices[4], Color);

    // Connecting edges
    AddLine(Vertices[0], Vertices[4], Color);
    AddLine(Vertices[1], Vertices[5], Color);
    AddLine(Vertices[2], Vertices[6], Color);
    AddLine(Vertices[3], Vertices[7], Color);
}


void URenderer::EndLineBatch(const FMatrix& ModelMatrix, const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix)
{
    if (!bLineBatchActive || !LineBatchData || !DynamicLineMesh || LineBatchData->Vertices.empty())
    {
        bLineBatchActive = false;
        return;
    }
    
    // Efficiently update dynamic mesh data (no buffer recreation!)
    if (!DynamicLineMesh->UpdateData(LineBatchData, RHIDevice->GetDeviceContext()))
    {
        bLineBatchActive = false;
        return;
    }
    
    // Set up rendering state
    UpdateConstantBuffer(ModelMatrix, ViewMatrix, ProjectionMatrix);
    PrepareShader(LineShader);
    
    // Render using dynamic mesh
    if (DynamicLineMesh->GetCurrentVertexCount() > 0 && DynamicLineMesh->GetCurrentIndexCount() > 0)
    {
        UINT stride = sizeof(FVertexSimple);
        UINT offset = 0;
        
        ID3D11Buffer* vertexBuffer = DynamicLineMesh->GetVertexBuffer();
        ID3D11Buffer* indexBuffer = DynamicLineMesh->GetIndexBuffer();
        
        RHIDevice->GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        RHIDevice->GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        RHIDevice->GetDeviceContext()->DrawIndexed(DynamicLineMesh->GetCurrentIndexCount(), 0, 0);
        
        // 라인 렌더링에 대한 DrawCall 통계 추가
        URenderingStatsCollector::GetInstance().IncrementDrawCalls();
    }
    
    bLineBatchActive = false;
}

void URenderer::ResetRenderStateTracking()
{
    LastMaterial = nullptr;
    LastShader = nullptr;
    LastTexture = nullptr;
}

void URenderer::ClearLineBatch()
{
    if (!LineBatchData) return;
    
    LineBatchData->Vertices.clear();
    LineBatchData->Color.clear();
    LineBatchData->Indices.clear();
    
    bLineBatchActive = false;
}

// TODO : Resource는 달라질 수 있으니, 나중에 좀 더 구조화하기
void URenderer::DrawFullScreenPass()                                                         
{                                                                                            
    ID3D11DeviceContext * Context = RHIDevice->GetDeviceContext();                           
    
    // Set up the pipeline for a full-screen pass                                            
    Context->IASetInputLayout(nullptr); // No input layout needed                            
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);                  
    
    // Unbind DepthStencilView Before Using Depth Buffer
    ID3D11RenderTargetView* RTV = static_cast<D3D11RHI*>(RHIDevice)->GetRenderTargetView();
    Context->OMSetRenderTargets(1, &RTV, NULL);

    // Bind the depth buffer as a shader resource for the pixel shader                       
    ID3D11ShaderResourceView* DepthSRV = static_cast<D3D11RHI*>(RHIDevice)->GetDepthBuffer();  
    Context->PSSetShaderResources(0, 1, &DepthSRV);
    ID3D11SamplerState* Sampler = static_cast<D3D11RHI*>(RHIDevice)->GetDefaultSampler();
    Context->PSSetSamplers(0, 1, &Sampler);

    // Draw a single triangle that covers the screen                                         
    Context->Draw(3, 0);                                                                     
    URenderingStatsCollector::GetInstance().IncrementDrawCalls();
    
    // Unbind the SRV to avoid holding a reference                                           
    ID3D11ShaderResourceView * NullSRV = nullptr;                                            
    Context->PSSetShaderResources(0, 1, &NullSRV);

    // Rebind DepthStencilView after using DepthBuffer
    ID3D11DepthStencilView* DepthStencilView = static_cast<D3D11RHI*>(RHIDevice)->GetDepthStencilView();
    Context->OMSetRenderTargets(1, &RTV, DepthStencilView);
}                                                                                            

void URenderer::RenderSceneDepth(float Near, float Far)
{
    static const UShader* SceneDepthShader = UResourceManager::GetInstance().Load<UMaterial>("SceneDepth.hlsl")->GetShader();

    PrepareShader(const_cast<UShader*>(SceneDepthShader));
    UpdateSceneDepthBuffer(Near, Far);

    // 렌더 상태 설정 (블렌딩, 깊이 스텐실)
    // 깊이 쓰기는 끄고, 깊이 테스트는 항상 통과 (기존 지오메트리 위에 그려지도록)
    OMSetDepthStencilState(EComparisonFunc::Always);

    DrawFullScreenPass();

    // 렌더 상태 복원 (선택 사항이지만 좋은 습관)
    OMSetDepthStencilState(EComparisonFunc::LessEqual); // 기본 깊이 스텐실 상태로 복원
}

