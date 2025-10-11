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

/**
* @brief OBB Component를 가진 Component 설정
*/
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

/**
* @brief 정점을 순회하면서 Local Min, Max를 계산합니다.
*/
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

/**
* @brief 필요 시에만 소유자(OwnerComponent)의 월드 행렬을 곱하여 월드 공간 OBB 계산
* 이렇게 하면 객체의 위치, 회전이 변경될 때마다 8개 꼭짓점을 모두 재계산할 필요 없이 월드 행렬만 갱신하면 됨
*/
FOrientedBound UOBoundingBoxComponent::GetWorldOrientedBox() const
{
    FVector Extent = GetExtent();
    
    if (OwnerComponent)
        return FOrientedBound(Extent, OwnerComponent->GetWorldMatrix());
    else
        return FOrientedBound(Extent, FMatrix::Identity());
}
/**
* @brief Local AABB의 Extent 반환
*/
FVector UOBoundingBoxComponent::GetExtent() const
{
    return (LocalMax - LocalMin) * 0.5f;
}
/**
* @brief World OBB를 계산한 후 라인을 그립니다. 
*/
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
/**
* @brief OBB를 감싸는 World AABB를 계산합니다.
* 로컬 AABB 정점 → (월드 변환) → 월드 OBB 정점 → (Min/Max 찾기) → 최종 월드 AABB
*/
FBound UOBoundingBoxComponent::GetWorldBound()
{
    // 월드 행렬 및 Local AABB의 절반 길이 가져오기 
	const FMatrix& WorldMatrix = OwnerComponent ? OwnerComponent->GetWorldMatrix() : FMatrix::Identity();
	const FVector LocalExtent = GetExtent();

	// 2. OBB의 로컬 공간 꼭짓점 8개를 정의 (Local AABB)
	const FVector LocalCorners[8] = {
		FVector(-LocalExtent.X, -LocalExtent.Y, -LocalExtent.Z),
		FVector(LocalExtent.X, -LocalExtent.Y, -LocalExtent.Z),
		FVector(-LocalExtent.X,  LocalExtent.Y, -LocalExtent.Z),
		FVector(LocalExtent.X,  LocalExtent.Y, -LocalExtent.Z),
		FVector(-LocalExtent.X, -LocalExtent.Y,  LocalExtent.Z),
		FVector(LocalExtent.X, -LocalExtent.Y,  LocalExtent.Z),
		FVector(-LocalExtent.X,  LocalExtent.Y,  LocalExtent.Z),
		FVector(LocalExtent.X,  LocalExtent.Y,  LocalExtent.Z)
	};

	// 첫번째 꼭짓점 -> 월드 OBB 정점으로 변환
	FVector FirstWorldCorner = TransformPosition(LocalCorners[0], WorldMatrix);
	FBound WorldAABB(FirstWorldCorner, FirstWorldCorner);

	// 나머지 7개 꼭짓점을 순회하며 AABB를 확장합니다.
	for (int i = 1; i < 8; ++i)
	{
		FVector WorldCorner = TransformPosition(LocalCorners[i], WorldMatrix);

		// FBound의 += 연산자를 사용하여 Min/Max를 자동으로 확장
		WorldAABB += FBound(WorldCorner, WorldCorner);
	}

	// 최종 계산된 월드 공간 AABB를 반환
	return WorldAABB;
}
/**
* @brief 현재 OBB와 AABB의 교차 검사 수행 (SAT)
* @param AABB 검사할 대상 AABB
*/
bool UOBoundingBoxComponent::IntersectsWithAABB(const FBound& AABB)
{
    FOrientedBound OBB = GetWorldOrientedBox();
    // OBB의 원점 얻기 (OBB Matrix로부터 이동 성분 취하면 됨)
    FVector OBBCenter = FVector(OBB.Orientation.M[3][0], OBB.Orientation.M[3][1], OBB.Orientation.M[3][2]);
    FVector AABBCenter = AABB.GetCenter();
    FVector AABBExtent = AABB.GetExtent();

    FMatrix OBBMatrix = OBB.Orientation;
    // OBB Matrix로부터 OBB Right, Up, Forward 기저 얻기 
    FVector OBBAxes[3] = {
        FVector(OBBMatrix.M[0][0], OBBMatrix.M[0][1], OBBMatrix.M[0][2]), // OBB Right
        FVector(OBBMatrix.M[1][0], OBBMatrix.M[1][1], OBBMatrix.M[1][2]), // OBB Up
        FVector(OBBMatrix.M[2][0], OBBMatrix.M[2][1], OBBMatrix.M[2][2])  // OBB Forward
    };

    // AABB의 월드 축 (unit vector)
    FVector AABBAxes[3] =
    {
        FVector(1.0f, 0.0f, 0.0f),
        FVector(0.0f, 1.0f, 0.0f),
        FVector(0.0f, 0.0f, 1.0f)
    };

    TArray<FVector> SatTestAxes;
    SatTestAxes.Add(AABBAxes[0]);
    SatTestAxes.Add(AABBAxes[1]);
    SatTestAxes.Add(AABBAxes[2]);
    SatTestAxes.Add(OBBAxes[0]);
    SatTestAxes.Add(OBBAxes[1]);
    SatTestAxes.Add(OBBAxes[2]);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3;++j)
        {
            FVector CrossProduct = FVector::Cross(AABBAxes[i], OBBAxes[j]);
            if (CrossProduct.SizeSquared() > KINDA_SMALL_NUMBER)
            {
                SatTestAxes.Add(CrossProduct.GetNormalized());
            }
        }
    }

    for (const FVector& Axis : SatTestAxes)
    {
        // SAT 테스트 할 축에 각 도형의 중심 투영
        float OBBCenterProj = FVector::Dot(OBBCenter, Axis);
        float AABBCenterProj = FVector::Dot(AABBCenter, Axis);

        // OBB의 반지름 : OBB의 각 축이 현재 테스트하는 축에 기여하는 길이 합산
        float OBBProjRadius =
            OBB.Extents.X * std::abs(FVector::Dot(OBBAxes[0], Axis)) +
            OBB.Extents.Y * std::abs(FVector::Dot(OBBAxes[1], Axis)) +
            OBB.Extents.Z * std::abs(FVector::Dot(OBBAxes[2], Axis));

        // AABB 반지름 : AABB의 각 축이 현재 테스트하는 축에 기여하는 길이 합산
        float AABBProjRadius =
            AABBExtent.X * std::abs(FVector::Dot(AABBAxes[0], Axis)) +
            AABBExtent.Y * std::abs(FVector::Dot(AABBAxes[1], Axis)) +
            AABBExtent.Z * std::abs(FVector::Dot(AABBAxes[2], Axis));

        float DistanceCenterProj = std::abs(OBBCenterProj - AABBCenterProj);
        // 중심 투영 거리 합산 > OBB 반지름 투영 + ABB 반지름 투영  
        // -> 분리 축 판정 완료(겹치지 않음 확정) -> 종료
        if (DistanceCenterProj > OBBProjRadius + AABBProjRadius)
        {
            return false;
        }
    }
    // 모든 축에서 겹친 경우 도형은 교차하는 것
    return true;
}
