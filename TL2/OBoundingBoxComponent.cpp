#include"pch.h"
#include "OBoundingBoxComponent.h"
#include "Box.h"
#include "Vector.h"

UOBoundingBoxComponent::UOBoundingBoxComponent(
    const std::vector<FVector>& Verts,
    USceneComponent* InOwner
)
{
    SetOwnerComponent(InOwner);
    SetFromVertices(Verts);
}

void UOBoundingBoxComponent::SetOwnerComponent(USceneComponent* InOwner)
{
    OwnerComponent = InOwner;
}

void UOBoundingBoxComponent::SetFromVertices(const std::vector<FVector>& Verts)
{
    if (Verts.empty()) return;

    LocalMin = LocalMax = Verts[0];
    for (auto& v : Verts)
    {
        LocalMin = LocalMin.ComponentMin(v);
        LocalMax = LocalMax.ComponentMax(v);
    }
}

void UOBoundingBoxComponent::SetFromVertices(const TArray<FNormalVertex>& Verts)
{
    if (Verts.empty()) return;

    LocalMin = LocalMax = Verts[0].pos;
    for (const auto& v : Verts)
    {
        LocalMin = LocalMin.ComponentMin(v.pos);
        LocalMax = LocalMax.ComponentMax(v.pos);
    }
}

FOrientedBound UOBoundingBoxComponent::GetWorldOrientedBox() const
{
    FVector Extent = GetExtent();
    
    if (OwnerComponent)
        return FOrientedBound(Extent, OwnerComponent->GetWorldMatrix());
    else
        return FOrientedBound(Extent, FMatrix::Identity());
}

FVector UOBoundingBoxComponent::GetExtent() const
{
    return (LocalMax - LocalMin) * 0.5f;
}

void UOBoundingBoxComponent::Render(URenderer* Renderer, const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix)
{
    (void)ViewMatrix;
    (void)ProjectionMatrix;

    TArray<FVector> Start;
    TArray<FVector> End;
    TArray<FVector4> Color;

    Bound = GetWorldOrientedBox();
    Bound.CreateLineData(Start, End, Color);
    Renderer->AddLines(Start, End, Color);
}
