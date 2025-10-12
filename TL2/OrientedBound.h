#pragma once

struct FOrientedBound
{
    FVector Extents;        // 각 축의 절반 길이
    FMatrix Orientation;    // OBB 월드 공간의 직교 축

    FOrientedBound() : Extents(FVector()), Orientation(FMatrix::Identity()) {}
    FOrientedBound(const FVector& InExtents, const FMatrix& InOrientation)
        : Extents(InExtents), Orientation(InOrientation) {
    }


    bool RayIntersects(const FVector& Origin, const FVector& Direction, float& Distance) const;

    void CreateLineData(
        OUT TArray<FVector>& Start,
        OUT TArray<FVector>& End,
        OUT TArray<FVector4>& Color
    );

    inline const static FVector4 LineColor{1.0f, 0.0f, 1.0f, 1.0f};
};