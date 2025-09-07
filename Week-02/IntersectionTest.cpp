#include "IntersectionTest.h"

bool CheckIntersectionRayBox(const FRay& Ray, const FAABB& LocalBox, const FTransform& T)
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

    for (int Axis = 0; Axis < 3; ++Axis)
    {
        float Origin = (Axis == 0) ? OriginLocal.X : (Axis == 1) ? OriginLocal.Y : OriginLocal.Z;
        float Direction = (Axis == 0) ? DirLocal.X : (Axis == 1) ? DirLocal.Y : DirLocal.Z;
        float BoxMin = (Axis == 0) ? LocalBox.Min.X : (Axis == 1) ? LocalBox.Min.Y : LocalBox.Min.Z;
        float BoxMax = (Axis == 0) ? LocalBox.Max.X : (Axis == 1) ? LocalBox.Max.Y : LocalBox.Max.Z;

        if (fabs(Direction) < MATH_EPSILON)
        {
            // 박스 면에 평행하므로 박스 내부에 없으면 교차 X
            if (Origin < BoxMin || Origin > BoxMax)
                return false;
        }
        else
        {
			// T1: Origin에서 가까운 slab까지의 거리, T2: Origin에서 먼 slab까지의 거리
            float T1 = (BoxMin - Origin) / Direction;
            float T2 = (BoxMax - Origin) / Direction;
            if (T1 > T2) std::swap(T1, T2);

			TMin = std::max(TMin, T1); // Max of Min
			TMax = std::min(TMax, T2); // Min of Max
        }
    }

	return TMax >= TMin && TMax >= 0.0f;
}