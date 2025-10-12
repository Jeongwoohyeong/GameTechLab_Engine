#include "pch.h"
#include "DecalComponent.h"

UDecalComponent::UDecalComponent() : OBB(DecalVolumeVertices, this)
{
    SetMaterial(MaterialPath);    
}

void UDecalComponent::ResetFadeProperties()
{
    FadeProperties = DefaultFadeProperties;
    ElapsedTime = 0.0f;
    bIsFadeOut = true;
}

UDecalComponent::~UDecalComponent()
{
    Material = nullptr;
}

void UDecalComponent::UpdateFade(float DeltaTime)
{
    if (!bIsFadeEnabled)
    {
        return;
    }

    if (FadeProperties.X > FLT_EPSILON)
    {
        ElapsedTime += DeltaTime;
        float Progress = ElapsedTime / FadeProperties.X;

        // 선형보간
        if (bIsFadeOut)
        {
            // fade out일 때 max에서 min까지 선형보간
            FadeProperties.W = FadeProperties.Z - (FadeProperties.Z - FadeProperties.Y) * Progress;
        }
        else
        {
            // min에서 max까지 선형보간
            FadeProperties.W = FadeProperties.Y + (FadeProperties.Z - FadeProperties.Y) * Progress;
        }

        // 진행도가 1 이상이면 fade in - fade out 전환
        // 진행시간 초기화
        if (Progress >= 1.0f)
        {
            ElapsedTime = 0.0f;
            bIsFadeOut = !bIsFadeOut;
        }
    }
}

void UDecalComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
    // 에디터에서만 보이도록 처리
    if (GetWorld() && !GetWorld()->IsPIEWorld())
    {
        //if (!bUsePerspectiveProjection)
        {
            // Ortho 모드: OBB 렌더링
            OBB.Render(Renderer, View, Proj);
        }
        //else
        //{
        //    // Perspective 모드: Frustum 선 렌더링
        //    RenderFrustumLines(Renderer);
        //}
    }
}
void UDecalComponent::RenderFrustumLines(URenderer* Renderer)
{
    // Frustum 파라미터
    const float Near = 0.01f;
    const float Far = 1.0f;
    const float halfAngleRad = DegreeToRadian(ProjectionFOV * 0.5f);
    const float tanHalfAngle = std::tan(halfAngleRad);

    // Near와 Far plane에서의 반경
    const float halfNear = tanHalfAngle * Near;
    const float halfFar = tanHalfAngle * Far;

    // Frustum 8개 꼭짓점 (Decal 로컬 공간, +Z forward)
    FVector nearCorners[4] = {
        FVector(Near - 0.5f, -halfNear,  halfNear),  // Near Top-Left
        FVector(Near - 0.5f, halfNear,  halfNear),  // Near Top-Right
        FVector(Near - 0.5f, halfNear, -halfNear),  // Near Bottom-Right
        FVector(Near - 0.5f, -halfNear, -halfNear)   // Near Bottom-Left
    };

    FVector farCorners[4] = {
        FVector(Far - 0.5f, -halfFar,  halfFar),     // Far Top-Left
        FVector(Far - 0.5f, halfFar,  halfFar),     // Far Top-Right
        FVector(Far - 0.5f, halfFar, -halfFar),     // Far Bottom-Right
        FVector(Far - 0.5f, -halfFar, -halfFar)      // Far Bottom-Left
    };

    // 월드 변환 행렬
    FMatrix worldMatrix = GetWorldMatrix();

    // 월드 좌표로 변환
    FVector nearWorld[4];
    FVector farWorld[4];

    for (int i = 0; i < 4; ++i)
    {
        nearWorld[i] = nearCorners[i] * worldMatrix;
        farWorld[i] = farCorners[i] * worldMatrix;
    }

    // Near plane 사각형 (4개 edge)
    for (int i = 0; i < 4; ++i)
    {
        Renderer->AddLine(nearWorld[i], nearWorld[(i + 1) % 4]);
    }

    // Far plane 사각형 (4개 edge)
    for (int i = 0; i < 4; ++i)
    {
        Renderer->AddLine(farWorld[i], farWorld[(i + 1) % 4]);
    }

    // Near-Far 연결선 (4개 edge)
    for (int i = 0; i < 4; ++i)
    {
        Renderer->AddLine(nearWorld[i], farWorld[i]);
    }
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

    // 야매 텍스처 로드(후에 제대로 텍스처 로드할 수 있도록 바꿔야 함)
    Material->Load(TexturePath, Renderer->GetRHIDevice()->GetDevice());
    
    Renderer->RSSetDefaultState();
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
        FadeProperties.W
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
}

void UDecalComponent::SetTexture(const FString& InTexturePath)
{
    TexturePath = InTexturePath;
}

void UDecalComponent::SetFadeProperties(const FVector4& InFadeProperties)
{
    FadeProperties = InFadeProperties;
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
    FMatrix proj = FMatrix::PerspectiveFovLH(FovYRadians, 1.0f, 0.01f, 1.0f);

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
    DuplicatedComponent->FadeProperties = this->FadeProperties;
    DuplicatedComponent->bUsePerspectiveProjection = this->bUsePerspectiveProjection;
    DuplicatedComponent->ProjectionFOV = this->ProjectionFOV;

    DuplicatedComponent->DuplicateSubObjects();
    return DuplicatedComponent;
}

void UDecalComponent::DuplicateSubObjects()
{
    Super_t::DuplicateSubObjects();
}

void UDecalComponent::Serialize(FObjectData* Data)
{
    FComponentData* ComponentData = dynamic_cast<FComponentData*>(Data);
    assert(ComponentData, "UStaticMeshComponent::Serialize got wrong data type.");

    USceneComponent::Serialize(Data);

    if (!TexturePath.empty())
    {
        ComponentData->Resource = TexturePath;
        UE_LOG("SaveScene: Decal Texture saved: %s", ComponentData->Resource.c_str());
    }
    else
    {
        UE_LOG("SaveScene: Decal has no Texture assigned");
    }
}

void UDecalComponent::DeSerialize(FObjectData* Data)
{
    FComponentData* ComponentData = dynamic_cast<FComponentData*>(Data);
    assert(ComponentData, "UStaticMeshComponent::DeSerialize got wrong data type.");

    USceneComponent::DeSerialize(Data);

    if (!ComponentData->Resource.empty())
    {
        TexturePath = ComponentData->Resource;
    }
}


void UDecalComponent::TickComponent(float DeltaTime)
{
    UPrimitiveComponent::TickComponent(DeltaTime);
    UpdateFade(DeltaTime);
}

void UDecalComponent::SetFadeEnabled(bool bIsEnable)
{
    this->bIsFadeEnabled = bIsEnable;
    // fade가 시작되면 세팅값 초기화
    if (bIsFadeEnabled)
    {
        ElapsedTime = 0.0f;
        bIsFadeOut = true;
    }
    // fade 종료 시 기본값으로 초기화
    else
    {
        FadeProperties = DefaultFadeProperties;
    }
    // Tick 활성화 제어
    SetTickEnabled(bIsFadeEnabled);
}
