#include "pch.h"
#include "DecalComponent.h"

UDecalComponent::UDecalComponent() : OBB(DecalVolumeVertices, this)
{
    SetMaterial("Decal.hlsl");
}

UDecalComponent::~UDecalComponent()
{
    Material = nullptr;
}

void UDecalComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
    OBB.Render(Renderer, View, Proj);
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
    if (StaticMesh)
    {
        // 야매 텍스처 로드(후에 제대로 텍스처 로드할 수 있도록 바꿔야 함)
        Material->Load("Editor/Decal/PointLight_64x.dds", Renderer->GetRHIDevice()->GetDevice());

        Renderer->RSSetNoCullState();
        Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqualReadOnly);
        Renderer->OMSetBlendState(true);

        FMatrix MeshWorld = StaticMeshComponent->GetWorldMatrix();

        Renderer->UpdateDecalConstantBuffer(
            MeshWorld * View * Proj,
            MeshWorld * \
            GetWorldMatrix().Inverse() * \
            DecalViewRotation * \
            DecalProjection
        );
        Renderer->PrepareShader(GetMaterial()->GetShader());
        Renderer->ProjectDecalToStaticMesh(
            this,
            StaticMesh,
            D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST
        );

        // 상태 원상복구
        Renderer->RSSetDefaultState();
        Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual);
        Renderer->OMSetBlendState(false);
    }
}