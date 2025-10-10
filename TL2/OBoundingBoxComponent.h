#pragma once
#include "ShapeComponent.h"
#include "OrientedBound.h"

struct FBox;

class UOBoundingBoxComponent :
    public UShapeComponent
{
    DECLARE_CLASS(UOBoundingBoxComponent,UShapeComponent)
public:
    UOBoundingBoxComponent() = default;

    UOBoundingBoxComponent(
        const std::vector<FVector>& Verts,
        USceneComponent* InOwner
    );

    void SetOwnerComponent(USceneComponent* InOwner);
    void SetFromVertices(const TArray<FNormalVertex>& Verts);

    // 주어진 로컬 버텍스들로부터 Min/Max 계산
    void SetFromVertices(const std::vector<FVector>& Verts);

    FOrientedBound GetWorldOrientedBox() const;

    // 로컬 공간에서의 Extent (절반 크기)
    FVector GetExtent() const;

	void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

private:
    FVector LocalMin;
    FVector LocalMax;
    FOrientedBound Bound;

    USceneComponent* OwnerComponent = nullptr;
};

