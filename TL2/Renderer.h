#pragma once
#include "BillboardComponent.h"
#include "DecalComponent.h"
#include "RHIDevice.h"
#include "LineDynamicMesh.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UFireBallComponent;
class UMeshComponent;
class URHIDevice;
class UShader;
class UStaticMesh;
struct FMaterialSlot;

class URenderer
{
public:
    URenderer(URHIDevice* InDevice);

    ~URenderer();

public:
	void BeginFrame();

    void PrepareShader(FShader& InShader);

    void PrepareShader(UShader* InShader);

    void OMSetBlendState(bool bIsChecked);

    void OMSetBlendState(EBlendMode BlendMode);

    void RSSetState(EViewModeIndex ViewModeIndex);

    void RSSetFrontCullState();

    void RSSetNoCullState();

    void RSSetDefaultState();

    void RSSetDecalState();

    void UpdateConstantBuffer(const FMatrix& ModelMatrix, const FMatrix& ViewMatrix, const FMatrix& ProjMatrix);

    void UpdateHighLightConstantBuffer(const uint32 InPicked, const FVector& InColor, const uint32 X, const uint32 Y, const uint32 Z, const uint32 Gizmo);

    void UpdateBillboardConstantBuffers(const FVector& pos, const FMatrix& ViewMatrix, const FMatrix& ProjMatrix, const FVector& CameraRight, const FVector& CameraUp);

    void UpdatePixelConstantBuffers(const FObjMaterialInfo& InMaterialInfo, bool bHasMaterial, bool bHasTexture);

    void UpdateColorBuffer(const FVector4& Color);

    void UpdateInvWorldBuffer(const FMatrix& InvWorldMatrix, const FMatrix& InvViewProjMatrix);

    void UpdateViewportBuffer(float StartX, float StartY, float SizeX, float SizeY);

    void DrawIndexedPrimitiveComponent(UStaticMesh* InMesh, D3D11_PRIMITIVE_TOPOLOGY InTopology, const TArray<FMaterialSlot>& InComponentMaterialSlots);

    void UpdateUVScroll(const FVector2D& Speed, float TimeSec);

    void UpdateDecalConstantBuffer(const FMatrix& InWorldMVP, const FMatrix& InDecalMVP, const float InAlpha);

    void UpdateFireBallConstantBuffer(const struct FireBallBufferType& InFireBallData);

    void UpdateHeightFogConstantBuffer(
        const FLinearColor& FogInscatteringColor,
        float FogDensity,
        float FogHeightFalloff,
        float StartDistance,
        float FogCutoffDistance,
        float FogMaxOpacity
    );

    void UpdateSceneDepthBuffer(float Near, float Far);

    void DrawFullScreenPass();
    void RenderSceneDepth(float Near, float Far);

    void ProjectDecalToStaticMesh(UDecalComponent* Comp, UStaticMesh* InMesh, D3D11_PRIMITIVE_TOPOLOGY InTopology);

    void DrawIndexedPrimitiveComponent(UTextRenderComponent* Comp, D3D11_PRIMITIVE_TOPOLOGY InTopology);
    void DrawIndexedPrimitiveComponent(UBillboardComponent* Comp,
                                       D3D11_PRIMITIVE_TOPOLOGY InTopology);
    void DrawIndexedPrimitiveComponent(UFireBallComponent* Comp,
                                        D3D11_PRIMITIVE_TOPOLOGY InTopology);

    void SetViewModeType(EViewModeIndex ViewModeIndex);
    // Batch Line Rendering System
    void BeginLineBatch();
    void AddLine(const FVector& Start, const FVector& End, const FVector4& Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f));
    void AddLines(const TArray<FVector>& StartPoints, const TArray<FVector>& EndPoints, const TArray<FVector4>& Colors);
    void DrawAABB(const FVector& Min, const FVector& Max, const FVector4& Color);

    void EndLineBatch(const FMatrix& ModelMatrix, const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix);
    void ClearLineBatch();

	void EndFrame();

    void OMSetDepthStencilState(EComparisonFunc Func);

    URHIDevice* GetRHIDevice() { return RHIDevice; }
private:
	URHIDevice* RHIDevice;

    // Batch Line Rendering System using UDynamicMesh for efficiency
    ULineDynamicMesh* DynamicLineMesh = nullptr;
    FMeshData* LineBatchData = nullptr;
    UShader* LineShader = nullptr;
    bool bLineBatchActive = false;
    static const uint32 MAX_LINES = 10000;  // Maximum lines per batch
    
    // 렌더링 통계를 위한 상태 추적
    UMaterial* LastMaterial = nullptr;
    UShader* LastShader = nullptr;
    UTexture* LastTexture = nullptr;

    /**
     * @brief 불필요한 API 호출을 막기 위해 마지막으로 바인딩된 상태를 캐싱합니다.
     */
    ID3D11Buffer* LastVertexBuffer = nullptr;
    ID3D11Buffer* LastIndexBuffer = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY LastPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11ShaderResourceView* LastTextureSRV = nullptr;

    void InitializeLineBatch();
    void ResetRenderStateTracking();
};

