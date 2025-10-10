#include"pch.h"
#include "OrientedBound.h"


bool FOrientedBound::RayIntersects(const FVector& Origin, const FVector& Direction, float& Distance) const
{
    const float Epsilon = 1e-6f;

    FVector RelativeOrigin = FVector() * Orientation - Origin;

    FVector AxisX = FVector(Orientation.M[0][0], Orientation.M[1][0], Orientation.M[2][0]);
    FVector AxisY = FVector(Orientation.M[0][1], Orientation.M[1][1], Orientation.M[2][1]);
    FVector AxisZ = FVector(Orientation.M[0][2], Orientation.M[1][2], Orientation.M[2][2]);

    float AxisDotOrigin[3] = {
        AxisX.Dot(RelativeOrigin),
        AxisY.Dot(RelativeOrigin),
        AxisZ.Dot(RelativeOrigin)
    };

    float AxisDotDirection[3] = {
        AxisX.Dot(Direction),
        AxisY.Dot(Direction),
        AxisZ.Dot(Direction)
    };

    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i)
    {
        float extent = (i == 0) ? Extents.X : (i == 1) ? Extents.Y : Extents.Z;

        if (std::abs(AxisDotDirection[i]) <= Epsilon)
        {
            if (std::abs(AxisDotOrigin[i]) > extent)
                return false;
        }
        else
        {
            float invAxisDotDirection = 1.0f / AxisDotDirection[i];
            float t1 = (AxisDotOrigin[i] - extent) * invAxisDotDirection;
            float t2 = (AxisDotOrigin[i] + extent) * invAxisDotDirection;

            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax) return false;
        }
    }

    if (tMax < 0.0f) return false;

    Distance = tMin >= 0.0f ? tMin : tMax;
    return true;
}

void FOrientedBound::CreateLineData(
    OUT TArray<FVector>& Start,
    OUT TArray<FVector>& End,
    OUT TArray<FVector4>& Color
)
{
    const uint32 VNUM = 8;

    // 8개 꼭짓점 정의
    FVector4 v[VNUM];

    v[0] = { -Extents.X, -Extents.Y, -Extents.Z, 1.0f };
    v[1] = { Extents.X, -Extents.Y, -Extents.Z, 1.0f };
    v[2] = { Extents.X, Extents.Y, -Extents.Z, 1.0f };
    v[3] = { -Extents.X, Extents.Y, -Extents.Z, 1.0f };
    v[4] = { -Extents.X, -Extents.Y, Extents.Z, 1.0f };
    v[5] = { Extents.X, -Extents.Y, Extents.Z, 1.0f };
    v[6] = { Extents.X, Extents.Y, Extents.Z, 1.0f };
    v[7] = { -Extents.X, Extents.Y, Extents.Z, 1.0f };

    for (int i = 0; i < VNUM; i++)
    {
        v[i] = v[i] * Orientation;
    }

    // --- 아래쪽 면 ---
    Start.Add(v[0].ToVec3()); End.Add(v[1].ToVec3()); Color.Add(LineColor);
    Start.Add(v[1].ToVec3()); End.Add(v[2].ToVec3()); Color.Add(LineColor);
    Start.Add(v[2].ToVec3()); End.Add(v[3].ToVec3()); Color.Add(LineColor);
    Start.Add(v[3].ToVec3()); End.Add(v[0].ToVec3()); Color.Add(LineColor);

    // --- 위쪽 면 ---
    Start.Add(v[4].ToVec3()); End.Add(v[5].ToVec3()); Color.Add(LineColor);
    Start.Add(v[5].ToVec3()); End.Add(v[6].ToVec3()); Color.Add(LineColor);
    Start.Add(v[6].ToVec3()); End.Add(v[7].ToVec3()); Color.Add(LineColor);
    Start.Add(v[7].ToVec3()); End.Add(v[4].ToVec3()); Color.Add(LineColor);

    // --- 옆면 기둥 ---
    Start.Add(v[0].ToVec3()); End.Add(v[4].ToVec3()); Color.Add(LineColor);
    Start.Add(v[1].ToVec3()); End.Add(v[5].ToVec3()); Color.Add(LineColor);
    Start.Add(v[2].ToVec3()); End.Add(v[6].ToVec3()); Color.Add(LineColor);
    Start.Add(v[3].ToVec3()); End.Add(v[7].ToVec3()); Color.Add(LineColor);
}