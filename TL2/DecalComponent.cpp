#include "pch.h"
#include "DecalComponent.h"

UDecalComponent::UDecalComponent() : OBB(DecalVolumeVertices, this)
{
    SetMaterial(MaterialPath);
}

void UDecalComponent::ResetFadeProperties()
{
    FadeProperties = DefaultFadeProperties;
}

UDecalComponent::~UDecalComponent()
{
    Material = nullptr;
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
        ComponentData->ResourceName = TexturePath;
        UE_LOG("SaveScene: Decal Texture saved: %s", ComponentData->ResourceName.c_str());
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

    if (!ComponentData->ResourceName.empty())
    {
        TexturePath = ComponentData->ResourceName;
    }
}

