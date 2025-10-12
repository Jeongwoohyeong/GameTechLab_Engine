#pragma once
#include "pch.h"
struct FBound
{
    FVector Min;
    FVector Max;

    FBound() : Min(FVector()), Max(FVector()) {}
    FBound(const FVector& InMin, const FVector& InMax) : Min(InMin), Max(InMax) {}
    /**
     * @brief 다른 FBound를 현재 FBound에 포함시키도록 확장하는 연산자
     * @param Other 포함시킬 다른 FBound
     * @return 확장된 자기 자신에 대한 참조
     */
    FBound& operator+=(const FBound& Other)
    {
        Min.X = std::min(Min.X, Other.Min.X);
        Min.Y = std::min(Min.Y, Other.Min.Y);
        Min.Z = std::min(Min.Z, Other.Min.Z);

        Max.X = std::max(Max.X, Other.Max.X);
        Max.Y = std::max(Max.Y, Other.Max.Y);
        Max.Z = std::max(Max.Z, Other.Max.Z);

        return *this;
    }
    // 센터 반환
    FVector GetCenter() const
    {
        return (Min + Max) * 0.5f;
    }
    // 박스의 절반 크기
    FVector GetExtent() const
    {
        return (Max - Min) * 0.5f;
    }

    // 표면적 계산 (SAH용)
    float GetSurfaceArea() const
    {
        FVector size = Max - Min;
        if (size.X <= 0.0f || size.Y <= 0.0f || size.Z <= 0.0f) return 0.0f;
        return 2.0f * (size.X * size.Y + size.Y * size.Z + size.Z * size.X);
    }
    bool IsInside(const FVector& Point) const
    {
        return (Point.X >= Min.X && Point.X <= Max.X &&
            Point.Y >= Min.Y && Point.Y <= Max.Y &&
            Point.Z >= Min.Z && Point.Z <= Max.Z);
    }
    bool IsIntersect(const FBound& Other) const
    {
        return (Min.X <= Other.Max.X && Max.X >= Other.Min.X &&
            Min.Y <= Other.Max.Y && Max.Y >= Other.Min.Y &&
            Min.Z <= Other.Max.Z && Max.Z >= Other.Min.Z);
    }

    bool RayIntersects(const FVector& Origin, const FVector& Direction, float& Distance) const
    {
        FVector invDir = FVector(1.0f / Direction.X, 1.0f / Direction.Y, 1.0f / Direction.Z);

        FVector t1 = (Min - Origin) * invDir;
        FVector t2 = (Max - Origin) * invDir;

        FVector tMin = t1.ComponentMin(t2);
        FVector tMax = t1.ComponentMax(t2);

        float tNear = std::max({tMin.X, tMin.Y, tMin.Z});
        float tFar = std::min({tMax.X, tMax.Y, tMax.Z});

        if (tNear > tFar || tFar < 0.0f) return false;

        Distance = (tNear < 0.0f) ? 0.0f : tNear;
        return true;
    }
    // Intersect AABB with AABB
    bool Intersects(const FBound& Other) const
    {
        if (Max.X < Other.Min.X || Min.X > Other.Max.X) return false;
        if (Max.Y < Other.Min.Y || Min.Y > Other.Max.Y) return false;
        if (Max.Z < Other.Min.Z || Min.Z > Other.Max.Z) return false;
        return true;
    }
        
    // FTransform으로 AABB를 변환
    FBound TransformBy(const FTransform& M) const
    {
    	FVector Corners[8] = {
    		FVector(Min.X, Min.Y, Min.Z),
    		FVector(Max.X, Min.Y, Min.Z),
    		FVector(Min.X, Max.Y, Min.Z),
    		FVector(Max.X, Max.Y, Min.Z),
    		FVector(Min.X, Min.Y, Max.Z),
    		FVector(Max.X, Min.Y, Max.Z),
    		FVector(Min.X, Max.Y, Max.Z),
    		FVector(Max.X, Max.Y, Max.Z)
    	};
    
    	FVector NewMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    	FVector NewMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    
    	for (int i = 0; i < 8; ++i)
    	{
    		FVector TransformedCorner = M.TransformPosition(Corners[i]);
    		NewMin.X = FMath::Min(NewMin.X, TransformedCorner.X);
    		NewMin.Y = FMath::Min(NewMin.Y, TransformedCorner.Y);
    		NewMin.Z = FMath::Min(NewMin.Z, TransformedCorner.Z);
    		NewMax.X = FMath::Max(NewMax.X, TransformedCorner.X);
    		NewMax.Y = FMath::Max(NewMax.Y, TransformedCorner.Y);
    		NewMax.Z = FMath::Max(NewMax.Z, TransformedCorner.Z);
    	}
    
    	return FBound(NewMin, NewMax);
    }
};

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