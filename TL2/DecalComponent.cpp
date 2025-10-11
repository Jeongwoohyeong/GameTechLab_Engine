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
    // OBB는 에디터에서만 보이도록 처리
    if (GetWorld() && !GetWorld()->IsPIEWorld())
    {
        OBB.Render(Renderer, View, Proj);
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

    Renderer->UpdateDecalConstantBuffer(
        MeshWorld * View * Proj,
        MeshWorld * \
        GetWorldMatrix().Inverse() * \
        DecalViewRotation * \
        DecalProjection,
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

UObject* UDecalComponent::Duplicate()
{
    UDecalComponent* DuplicatedComponent = Cast<UDecalComponent>(NewObject(GetClass()));
    CopyCommonProperties(DuplicatedComponent);

    DuplicatedComponent->Material = this->Material;
    DuplicatedComponent->TexturePath = this->TexturePath;

    DuplicatedComponent->DuplicateSubObjects();
    return DuplicatedComponent;
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
