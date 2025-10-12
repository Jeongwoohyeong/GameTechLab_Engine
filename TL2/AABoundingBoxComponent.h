#pragma once
#include "ShapeComponent.h"
#include "OrientedBound.h"

class ULine;
struct FOrientedBound;
class UAABoundingBoxComponent :
    public UShapeComponent
{
    DECLARE_CLASS(UAABoundingBoxComponent, UShapeComponent)
public:
    UAABoundingBoxComponent();

    // 주어진 로컬 버텍스들로부터 Min/Max 계산
    void SetFromVertices(const TArray<FVector>& Verts);
    void SetFromVertices(const TArray<FNormalVertex>& Verts);
    void SetMinMax(const FBound& Bound);
    void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;
    void TickComponent(float DeltaTime) override;
    // 월드 좌표계에서의 AABB 반환
    FBound GetWorldBoundFromCube();
    FBound GetWorldBoundFromSphere() const;
    const FBound* GetFBound() const;
    // OBB 관련 함수들
    FOrientedBound GetWorldOrientedBound() const;
    bool RayIntersectsOBB(const FVector& Origin, const FVector& Direction, float& Distance) const;

    TArray<FVector4> GetLocalCorners() const;

    void SetPrimitiveType(EPrimitiveType InType) { PrimitiveType = InType; }
    void SetLineColor(FVector4 InLineColor) { LineColor = InLineColor; };

    // Duplicate
    UObject* Duplicate() override;
	//void DuplicateSubObjects() override;

private:
    void CreateLineData(
        const FVector& Min, const FVector& Max,
        OUT TArray<FVector>& Start,
        OUT TArray<FVector>& End,
        OUT TArray<FVector4>& Color);

    FBound Bound;
    FVector LocalMin;
    FVector LocalMax;
    FVector4 LineColor;
    EPrimitiveType PrimitiveType = EPrimitiveType::Default;

    // AABB 계산 캐싱용
    FMatrix LastWorldMat = FMatrix::Zero();
    FBound WorldBound;
};

