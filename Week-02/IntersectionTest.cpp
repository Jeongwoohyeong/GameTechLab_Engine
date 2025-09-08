#include "IntersectionTest.h"

static bool IntersectImpl(const FRay& Ray, const FAABB& LocalBox, FTransform T, FHit* OutHitOpt)
{
    // 1. 월드좌표계 Ray를 로컬좌표계로 변환
    FMatrix InvMat;
    if (!T.TryGetInverseMatrix(InvMat))
        return false; // 스케일 중 0이 포함되어 있는 경우

    // Ray의 Origin 변환 (점)
    FVector4 OriginLocal4 = FVector4::MakePoint(Ray.Origin) * InvMat;
    FVector OriginLocal = FVector(OriginLocal4.X, OriginLocal4.Y, OriginLocal4.Z);

    // Ray의 Direction 변환 (벡터이므로 평행이동 제외)
    FVector4 DirectionLocal4 = FVector4::MakeVector(Ray.Direction) * InvMat;
    FVector DirLocal = FVector(DirectionLocal4.X, DirectionLocal4.Y, DirectionLocal4.Z).GetNormalized();

    // 2. Slab Method로 AABB 교차 판정
    float TMin = -FLT_MAX;
    float TMax = FLT_MAX;

    // 2. Slab Method로 AABB 교차 판정
    for (int Axis = 0; Axis < 3; ++Axis)
    {
        float Origin = (Axis == 0) ? OriginLocal.X : (Axis == 1) ? OriginLocal.Y : OriginLocal.Z;
        float Dir = (Axis == 0) ? DirLocal.X : (Axis == 1) ? DirLocal.Y : DirLocal.Z;
        float BMin = (Axis == 0) ? LocalBox.Min.X : (Axis == 1) ? LocalBox.Min.Y : LocalBox.Min.Z;
        float BMax = (Axis == 0) ? LocalBox.Max.X : (Axis == 1) ? LocalBox.Max.Y : LocalBox.Max.Z;

		// 박스 면에 평행한 경우
        if (fabs(Dir) < MATH_EPSILON)
        {
            if (Origin < BMin || Origin > BMax)
            {
                return false; // 광선이 박스 바깥에 있으면 교차 X
            }
			continue; // 광선이 박스를 통과하지만 이 축에 대해서는 교차 X
        }

        float T1 = (BMin - Origin) / Dir;
        float T2 = (BMax - Origin) / Dir;
        if (T1 > T2) std::swap(T1, T2);

        TMin = std::max(TMin, T1);
        TMax = std::min(TMax, T2);
        if (TMax < TMin) return false;
    }

    if (TMax < 0.0f) return false; // 박스가 레이 뒤쪽

    if (OutHitOpt)
    {
        FHit& Hit = *OutHitOpt;
        Hit.TEnter = TMin;
        Hit.TExit = TMax;
        Hit.bIsInside = (TMin < 0.0f);
        Hit.T = Hit.bIsInside ? TMax : TMin;

        // 로컬 히트 포인트
        Hit.PointLocal = OriginLocal + DirLocal * Hit.T;
    }

    return true;
}

bool CheckIntersectionRayBox(const FRay& Ray, const FAABB& LocalBox, const FTransform T)
{
    return IntersectImpl(Ray, LocalBox, T, nullptr);
}

bool CheckIntersectionRayBox(const FRay& Ray, const FAABB& LocalBox, const FTransform T, FHit& OutHit)
{
    return IntersectImpl(Ray, LocalBox, T, &OutHit);
}