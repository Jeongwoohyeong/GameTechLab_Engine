#pragma once
#include "Vector.h"
#include "UEContainer.h"
#include"AABoundingBoxComponent.h"
#include "Actor.h"
#include <cmath>

struct FBound;
class URenderer; // Forward Declaration for the renderer

// 최적화된 Ray-AABB 교차 검사를 위한 구조체
struct alignas(16) FOptimizedRay
{
    FVector Origin;
    FVector Direction;
    FVector InverseDirection;  // 1.0f / Direction (division 제거용)
    int Sign[3];              // Direction의 부호 (branchless용)

    FOptimizedRay(const FVector& InOrigin, const FVector& InDirection)
        : Origin(InOrigin), Direction(InDirection)
    {
        // InverseDirection 계산 (0으로 나누기 방지)
        InverseDirection.X = (std::abs(InDirection.X) < KINDA_SMALL_NUMBER) ?
                            (InDirection.X < 0.0f ? -1e30f : 1e30f) : 1.0f / InDirection.X;
        InverseDirection.Y = (std::abs(InDirection.Y) < KINDA_SMALL_NUMBER) ?
                            (InDirection.Y < 0.0f ? -1e30f : 1e30f) : 1.0f / InDirection.Y;
        InverseDirection.Z = (std::abs(InDirection.Z) < KINDA_SMALL_NUMBER) ?
                            (InDirection.Z < 0.0f ? -1e30f : 1e30f) : 1.0f / InDirection.Z;

        // Sign 계산 (branchless slab method용)
        Sign[0] = (InverseDirection.X < 0.0f) ? 1 : 0;
        Sign[1] = (InverseDirection.Y < 0.0f) ? 1 : 0;
        Sign[2] = (InverseDirection.Z < 0.0f) ? 1 : 0;
    }

    // 최적화된 Ray-AABB 교차 검사 (branchless slab method)
    inline bool IntersectAABB(const FBound& Box, float& OutTNear) const
    {
        // AABB의 min/max를 배열로 접근하기 위한 설정
        FVector BoundsArray[2] = { Box.Min, Box.Max };

        float tmin = (BoundsArray[Sign[0]].X - Origin.X) * InverseDirection.X;
        float tmax = (BoundsArray[1 - Sign[0]].X - Origin.X) * InverseDirection.X;

        float tymin = (BoundsArray[Sign[1]].Y - Origin.Y) * InverseDirection.Y;
        float tymax = (BoundsArray[1 - Sign[1]].Y - Origin.Y) * InverseDirection.Y;

        // Branch 없이 min/max 계산
        tmin = FMath::Max(tmin, tymin);
        tmax = FMath::Min(tmax, tymax);

        float tzmin = (BoundsArray[Sign[2]].Z - Origin.Z) * InverseDirection.Z;
        float tzmax = (BoundsArray[1 - Sign[2]].Z - Origin.Z) * InverseDirection.Z;

        tmin = FMath::Max(tmin, tzmin);
        tmax = FMath::Min(tmax, tzmax);

        // 교차 여부 및 거리 반환
        OutTNear = tmin;
        return (tmax >= tmin) && (tmax >= 0.0f);
    }
};

// BVH 노드 구조체
struct FBVHNode
{
    FBound BoundingBox; // 노드가 이루는 AABB

    // Leaf Node 전용 Data
    int FirstPrim;  // 첫 번째 프리미티브 인덱스
    int PrimCount;  // 프리미티브 개수

    // Internal Node 전용 Data
    int LeftChild;   // 왼쪽 자식 노드 인덱스
    int RightChild;  // 오른쪽 자식 노드 인덱스

    // 생성자
    FBVHNode()
        : FirstPrim(-1), PrimCount(0), LeftChild(-1), RightChild(-1)
    {
    }

    // 리프 노드인지 확인
    bool IsLeaf() const { return PrimCount > 0; }
};

// 프리미티브의 World AABB와 포인터를 저장
struct FPrimitiveBounds
{
    UPrimitiveComponent* Primitive;
    FBound Bounds;  // World AABB
    FVector Center;

    FPrimitiveBounds() : Primitive(nullptr) {}
    FPrimitiveBounds(UPrimitiveComponent* InPrimitive, const FBound& InBounds)
        : Primitive(InPrimitive), Bounds(InBounds)
    {
        Center = (InBounds.Min + InBounds.Max) * 0.5f;
    }
};

// 고성능 BVH 구현
class FBVH
{
public:
    FBVH();
    ~FBVH();

    // 프리미티브 배열로부터 BVH 구축
    void Build(const TArray<UPrimitiveComponent*>& Primitives);
    void Refit();
    void Clear();

    // 빠른 레이 교차 검사 - 가장 가까운 프리미티브 반환
    UPrimitiveComponent* Intersect(const FVector& RayOrigin, const FVector& RayDirection, float& OutDistance) const;

    // AABB와 교차하는 모든 프리미티브 찾기
    void IntersectAABB(const FBound& QueryAABB, TArray<UPrimitiveComponent*>& OutPrimitives) const;

    // BVH 구조를 디버깅용 라인으로 렌더링
    void Render(URenderer* Renderer) const;

    // 통계 정보
    int GetNodeCount() const { return Nodes.Num(); }
    int GetPrimitiveCount() const { return PrimitiveBounds.Num(); }
    int GetMaxDepth() const { return MaxDepth; }

    // 렌더링을 위한 노드 접근
    const TArray<FBVHNode>& GetNodes() const { return Nodes; }

    static float SurfaceArea(const FBound& b);
private:
    TArray<FBVHNode> Nodes;
    TArray<FPrimitiveBounds> PrimitiveBounds;
    TArray<int> PrimitiveIndices; // 정렬된 프리미티브 인덱스

    int MaxDepth;

    // 재귀 구축 함수
    int BuildRecursive(int FirstPrim, int PrimCount, int Depth = 0);
    // Refit 위한 재귀 헬퍼
    FBound RefitRecursive(int NodeIndex);
    // 경계 박스 계산
    FBound CalculateBounds(int FirstPrim, int PrimCount) const;
    FBound CalculateCentroidBounds(int FirstPrim, int PrimCount) const;

    // Surface Area Heuristic을 이용한 최적 분할
    int FindBestSplit(int FirstPrim, int PrimCount, int& OutAxis, float& OutSplitPos);
    float CalculateSAH(int FirstPrim, int LeftCount, int RightCount, const FBound& ParentBounds) const;

    // 프리미티브 분할
    int PartitionPrimitives(int FirstPrim, int PrimCount, int Axis, float SplitPos);

    bool IntersectNode(int NodeIndex, const FOptimizedRay& Ray, float& InOutDistance, UPrimitiveComponent*& OutPrimitive) const;
    void IntersectAABBRecursive(int NodeIndex, const FBound& QueryAABB, TArray<UPrimitiveComponent*>& OutPrimitives) const;

    // 프리미티브와의 교차 검사
    bool IntersectPrimitive(const UPrimitiveComponent* Primitive, const FVector& RayOrigin, const FVector& RayDirection, float& OutDistance) const;

    // Render를 위한 재귀 헬퍼
    void RenderRecursive(URenderer* Renderer, int NodeIndex, int Depth) const;

    // 상수
    static const int MaxPrimsPerLeaf = 4;
    static const int MaxBVHDepth = 32;
};