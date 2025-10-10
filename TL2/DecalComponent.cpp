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
        Renderer->RSSetNoCullState();
        Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqualReadOnly);

        FMatrix MeshWorld = StaticMeshComponent->GetWorldMatrix();

        Renderer->UpdateDecalConstantBuffer(
            MeshWorld * View * Proj,
            MeshWorld * GetWorldMatrix().Inverse() * DecalProjection
        );
        Renderer->PrepareShader(GetMaterial()->GetShader());
        Renderer->ProjectDecalToStaticMesh(
            StaticMesh,
            D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST
        );

        // 상태 원상복구
        Renderer->RSSetDefaultState();
        Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual);
    }
}