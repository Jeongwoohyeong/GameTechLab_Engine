#include "pch.h"
#include "DecalComponent.h"

UDecalComponent::UDecalComponent() : OBB(DecalVolumeVertices, this)
{
    SetMaterial(MaterialPath);    
}

void UDecalComponent::ResetFadeProperties()
{
    bool bIsEnabled = CurrentFadeProperty.bIsFadeEnabled;
    CurrentFadeProperty = DefaultFadeProperty;
    CurrentFadeProperty.bIsFadeEnabled = bIsEnabled;
    ElapsedTime = 0.0f;
}

UDecalComponent::~UDecalComponent()
{
    Material = nullptr;
}

void UDecalComponent::UpdateFade(float DeltaTime)
{
    if (!CurrentFadeProperty.bIsFadeEnabled)
    {
        return;
    }

    if (CurrentFadeProperty.AlphaProperties.X <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    if (CurrentFadeProperty.bIsFadeStart)
    {
        ElapsedTime += DeltaTime;
        float Progress = ElapsedTime / CurrentFadeProperty.AlphaProperties.X;
        float ClampedProgress = std::min(Progress, 1.0f);
        EFadeTypes Type = CurrentFadeProperty.Type;
        switch (Type)
        {
        case EFadeTypes::FadeIn:
            {
                if (!CurrentFadeProperty.bIsLoop)
                {
                    CurrentFadeProperty.AlphaProperties.W = CurrentFadeProperty.AlphaProperties.Y;                    
                }
                CurrentFadeProperty.AlphaProperties.W = CurrentFadeProperty.AlphaProperties.Y +
                        (CurrentFadeProperty.AlphaProperties.Z -
                            CurrentFadeProperty.AlphaProperties.Y) *
                        ClampedProgress;
 
                break;
            }
        case EFadeTypes::FadeOut:
            {
                if (!CurrentFadeProperty.bIsLoop)
                {
                    CurrentFadeProperty.AlphaProperties.W = CurrentFadeProperty.AlphaProperties.Y;                    
                }
                CurrentFadeProperty.AlphaProperties.W = CurrentFadeProperty.AlphaProperties.Z -
                        (CurrentFadeProperty.AlphaProperties.Z -
                            CurrentFadeProperty.AlphaProperties.Y) *
                        ClampedProgress;
                
                break;
            }
        default:
            break;
        }

        if (Progress >= 1.0f)
        {
            ElapsedTime = 0.0f;
            if (CurrentFadeProperty.bIsLoop)
            {
                CurrentFadeProperty.Type = (++Type) % EFadeTypes::FadeLoop;
            }
            else
            {
                CurrentFadeProperty.bIsFadeStart = false;
                CurrentFadeProperty.AlphaProperties.W = 1.0f;
            }            
        }
    }    
}

void UDecalComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
    // 에디터에서만 보이도록 처리
    if (GetWorld() && !GetWorld()->IsPIEWorld())
    {
        if (!bUsePerspectiveProjection)
        {
            // Ortho 모드: OBB 렌더링
            OBB.Render(Renderer, View, Proj);
        }
        else
        {
            // Perspective 모드: Frustum 선 렌더링
            RenderFrustumLines(Renderer);
        }
    }
}
void UDecalComponent::RenderFrustumLines(URenderer* Renderer)
{
    // Define the 5 corners of the Frustum
    FVector NDCCorners[5] = {
        FVector(0.0f, 0.0f, 0.0f), // Near-Bottom-Left
        FVector(-1.0f, -1.0f, 1.0f), // Far-Bottom-Left
        FVector( 1.0f, -1.0f, 1.0f), // Far-Bottom-Right
        FVector( 1.0f,  1.0f, 1.0f), // Far-Top-Right
        FVector(-1.0f,  1.0f, 1.0f)  // Far-Top-Left
    };

    // Get the matrix that transforms from Decal Clip Space (NDC) to World Space
    FMatrix ClipToLocal = (DecalViewAdjustMatrix * GetDecalPerspectiveProjection(ProjectionFOV)).Inverse();
    FMatrix DecalClipToWorld = ClipToLocal * GetWorldMatrix();

    // Transform NDC corners to world space
    FVector WorldCorners[5];
    for (int i = 0; i < 5; ++i)
    {
        FVector4 Point(NDCCorners[i].X, NDCCorners[i].Y, NDCCorners[i].Z, 1.0f);
        FVector4 TransformedPoint = Point * DecalClipToWorld;
        
        // Perform perspective divide
        if (abs(TransformedPoint.W) > 1e-6f)
        {
            WorldCorners[i] = FVector(TransformedPoint.X / TransformedPoint.W, TransformedPoint.Y / TransformedPoint.W, TransformedPoint.Z / TransformedPoint.W);
        }
        else
        {
            // Fallback for w ~ 0
            WorldCorners[i] = FVector(TransformedPoint.X, TransformedPoint.Y, TransformedPoint.Z);
        }
    }

    // Draw lines for the far plane
    Renderer->AddLine(WorldCorners[1], WorldCorners[2]);
    Renderer->AddLine(WorldCorners[2], WorldCorners[3]);
    Renderer->AddLine(WorldCorners[3], WorldCorners[4]);
    Renderer->AddLine(WorldCorners[4], WorldCorners[1]);

    // Draw lines connecting near and far planes
    Renderer->AddLine(WorldCorners[0], WorldCorners[1]);
    Renderer->AddLine(WorldCorners[0], WorldCorners[2]);
    Renderer->AddLine(WorldCorners[0], WorldCorners[3]);
    Renderer->AddLine(WorldCorners[0], WorldCorners[4]);
}

void UDecalComponent::ProjectDecal
(
    URenderer* Renderer,
    UStaticMeshComponent* StaticMeshComponent,
    const FMatrix& View,
    const FMatrix& Proj
)
{
    UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
    if (!StaticMesh)
    {
        return;
    }

    if (!Material || !Material->GetShader())
    {
        // 렌더링 직전에 Material 또는 Shader가 유효하지 않으면 드로우 콜이 스킵됩니다.
        UE_LOG("ProjectDecal: Skipping render! Material or Shader is NULL! (Mat: %p, Shader: %p)", Material, Material ? Material->GetShader() : nullptr);
        return; // 여기서 렌더링을 중단
    }
    
    // 야매 텍스처 로드(후에 제대로 텍스처 로드할 수 있도록 바꿔야 함)
    Material->Load(TexturePath, Renderer->GetRHIDevice()->GetDevice());

    //Renderer->RSSetDefaultState();
    Renderer->RSSetDecalState();
    Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqualReadOnly);
    Renderer->OMSetBlendState(true);

    FMatrix MeshWorld = StaticMeshComponent->GetWorldMatrix();

    // 모드에 따라 적절한 Projection 행렬 선택
    FMatrix DecalProj = bUsePerspectiveProjection
        ? GetDecalPerspectiveProjection(ProjectionFOV)
        : GetDecalOrthoProjection();
    
    Renderer->UpdateDecalConstantBuffer(
        MeshWorld * View * Proj,
        MeshWorld * \
        GetWorldMatrix().Inverse() * \
        DecalViewAdjustMatrix * \
        DecalProj,
        CurrentFadeProperty.AlphaProperties.W
    );
    Renderer->PrepareShader(GetMaterial()->GetShader());
    Renderer->ProjectDecalToStaticMesh(
        this,
        StaticMesh,
        D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    // 상태 원상복구
    Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual);
    Renderer->OMSetBlendState(false);
    Renderer->RSSetDefaultState();
}

void UDecalComponent::SetTexture(const FString& InTexturePath)
{
    TexturePath = InTexturePath;
}


FMatrix UDecalComponent::GetDecalOrthoProjection()
{
    return FMatrix::OrthoLH(1.0f, 1.0f, 0.01f, 1.0f);
    //return FMatrix(
    //    2.0f, 0.0f, 0.0f, 0.0f,  // x: 2배 스케일
    //    0.0f, 2.0f, 0.0f, 0.0f,  // y: 2배 스케일
    //    0.0f, 0.0f, 2.0f, 0.0f,  // z: 2배 스케일
    //    0.0f, 0.0f, 0.0f, 1.0f
    //);
}

FMatrix UDecalComponent::GetDecalPerspectiveProjection(float FovYDegrees)
{
    // FOV를 라디안으로 변환
    float FovYRadians = DegreeToRadian(FovYDegrees);

    // 기본 perspective 행렬 생성
    FMatrix proj = FMatrix::PerspectiveFovLH(FovYRadians, 1.0f, 0.001f, 1.0f);

    // Decal Volume 크기 보정:
    // - Decal Volume: far (z=1)에서 반경 0.5 (고정)
    // - Perspective frustum: far에서 반경 tan(FOV/2)
    // - 보정 스케일: 2 * tan(FOV/2) / 1.0 = 2 * tan(FOV/2)
    // 이렇게 하면 Decal Volume [−0.5, 0.5]가 정확히 NDC [−1, 1]로 매핑됨
    float correctionScale = 2.0f * std::tan(FovYRadians * 0.5f);
    proj.M[0][0] *= correctionScale;  // X 스케일 조정
    proj.M[1][1] *= correctionScale;  // Y 스케일 조정

    return proj;
}

UObject* UDecalComponent::Duplicate()
{
    UDecalComponent* DuplicatedComponent = Cast<UDecalComponent>(NewObject(GetClass()));
    CopyCommonProperties(DuplicatedComponent);

    DuplicatedComponent->Material = this->Material;
    DuplicatedComponent->TexturePath = this->TexturePath;
    DuplicatedComponent->bUsePerspectiveProjection = this->bUsePerspectiveProjection;
    DuplicatedComponent->ProjectionFOV = this->ProjectionFOV;
    DuplicatedComponent->CurrentFadeProperty = this->CurrentFadeProperty;
    DuplicatedComponent->ElapsedTime = this->ElapsedTime;    

    DuplicatedComponent->DuplicateSubObjects();
    return DuplicatedComponent;
}

void UDecalComponent::DuplicateSubObjects()
{
    Super_t::DuplicateSubObjects();
}

void UDecalComponent::Serialize(FObjectData* Data)
{
    FDecalComponentData* ComponentData = dynamic_cast<FDecalComponentData*>(Data);
    assert(ComponentData, "UStaticMeshComponent::Serialize got wrong data type.");

    USceneComponent::Serialize(Data);

    if (!TexturePath.empty())
    {
        ComponentData->Texture = TexturePath;
        UE_LOG("SaveScene: Decal Texture saved: %s", ComponentData->Texture.c_str());
    }
    else
    {
        UE_LOG("SaveScene: Decal has no Texture assigned");
    }


    ComponentData->FadeType = ToUnderlying(CurrentFadeProperty.Type);
    ComponentData->Duration = CurrentFadeProperty.AlphaProperties.X;
    ComponentData->Min = CurrentFadeProperty.AlphaProperties.Y;
    ComponentData->Max = CurrentFadeProperty.AlphaProperties.Z;
    ComponentData->Alpha = CurrentFadeProperty.AlphaProperties.W;
    ComponentData->bIsFadeEnabled = CurrentFadeProperty.bIsFadeEnabled;
    ComponentData->bIsFadeStart = CurrentFadeProperty.bIsFadeStart;
    ComponentData->bIsLoop = CurrentFadeProperty.bIsLoop;

    ComponentData->ElapsedTime = ElapsedTime;
}

void UDecalComponent::DeSerialize(FObjectData* Data)
{
    FDecalComponentData* ComponentData = dynamic_cast<FDecalComponentData*>(Data);
    assert(ComponentData, "UStaticMeshComponent::DeSerialize got wrong data type.");

    UE_LOG("decal");
    USceneComponent::DeSerialize(Data);

    if (!ComponentData->Texture.empty())
    {
        TexturePath = ComponentData->Texture;
    }
    
    CurrentFadeProperty.Type = static_cast<EFadeTypes>(ComponentData->FadeType);
    CurrentFadeProperty.AlphaProperties.X = ComponentData->Duration;
    CurrentFadeProperty.AlphaProperties.Y = ComponentData->Min;
    CurrentFadeProperty.AlphaProperties.Z = ComponentData->Max;
    CurrentFadeProperty.AlphaProperties.W = ComponentData->Alpha;
    CurrentFadeProperty.bIsFadeEnabled = ComponentData->bIsFadeEnabled;
    CurrentFadeProperty.bIsFadeStart = ComponentData->bIsFadeStart;
    CurrentFadeProperty.bIsLoop = ComponentData->bIsLoop;

    ElapsedTime = ComponentData->ElapsedTime;

}

void UDecalComponent::TickComponent(float DeltaTime)
{    
    UPrimitiveComponent::TickComponent(DeltaTime);
}
