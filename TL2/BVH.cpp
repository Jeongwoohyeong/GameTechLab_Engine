#include "pch.h"
#include "BVH.h"
#include "StaticMeshActor.h"
#include "Picking.h"
#include "PickingTimer.h"
#include "UI/GlobalConsole.h"
#include "Renderer.h" // For URenderer and DrawAABB
#include "PrimitiveComponent.h"
#include <algorithm>
#include <cfloat>


FBVH::FBVH() : MaxDepth(0)
{
}

FBVH::~FBVH()
{
    Clear();
}

void FBVH::Build(const TArray<UPrimitiveComponent*>& Primitives)
{
    TStatId BVHBuildStatId;
    FScopeCycleCounter BVHBuildTimer(BVHBuildStatId);

    Clear();

    if (Primitives.Num() == 0)
        return;

    // 1. 프리미티브들의 AABB 정보 수집
    PrimitiveBounds.Reserve(Primitives.Num());
    PrimitiveIndices.Reserve(Primitives.Num());

    for (int i = 0; i < Primitives.Num(); ++i)
    {
        UPrimitiveComponent* Prim = Primitives[i];
        if (!Prim || !Prim->IsActive()) 
            continue;
        // AABB와 OBB는 BVH에 포함 안시킴
        if (UAABoundingBoxComponent* AABBComp = Cast<UAABoundingBoxComponent>(Prim))
            continue;
        if (UOBoundingBoxComponent* OBBComp = Cast<UOBoundingBoxComponent>(Prim))
            continue;

        // World AABB
        FBound WorldBound = Prim->GetWorldBound();
        
        // Primitive의 Bound 목록 및 인덱스에 추가
        FPrimitiveBounds PB(Prim, WorldBound);
        PrimitiveBounds.Add(PB);
        PrimitiveIndices.Add(PrimitiveBounds.Num() - 1);
    }

    if (PrimitiveBounds.Num() == 0)
        return;

    // 2. 노드 배열 예약 (최악의 경우 2*N-1개 노드)
    Nodes.Reserve(PrimitiveBounds.Num() * 2);

    // 3. 재귀적으로 BVH 구축
    MaxDepth = 0;
    int RootIndex = BuildRecursive(0, PrimitiveBounds.Num(), 0);

    uint64_t BuildCycles = BVHBuildTimer.Finish();
    double BuildTimeMs = FPlatformTime::ToMilliseconds(BuildCycles);

    char buf[256];
    sprintf_s(buf, "[BVH] Built for %d primitives, %d nodes, depth %d (Time: %.3fms)\n",
        PrimitiveBounds.Num(), Nodes.Num(), MaxDepth, BuildTimeMs);
    UE_LOG(buf);
}

/**
* @brief 두 AABB를 합친 AABB를 만든다.
*/
static inline FBound Union(const FBound& A, const FBound& B)
{
    FBound Out;
    Out.Min.X = FMath::Min(A.Min.X, B.Min.X);
    Out.Min.Y = FMath::Min(A.Min.Y, B.Min.Y);
    Out.Min.Z = FMath::Min(A.Min.Z, B.Min.Z);

    Out.Max.X = FMath::Max(A.Max.X, B.Max.X);
    Out.Max.Y = FMath::Max(A.Max.Y, B.Max.Y);
    Out.Max.Z = FMath::Max(A.Max.Z, B.Max.Z);
    return Out;
}

/**
* @brief 
*/
void FBVH::Refit()
{
    if (Nodes.Num() == 0)
    {
        return;
    }
    // 저장된 Primitive의 Bound 목록으로부터 실제 Bound 갱신
    for (FPrimitiveBounds& PB : PrimitiveBounds)
    {
        if (PB.Primitive)
        {
            PB.Bounds = PB.Primitive->GetWorldBound();
        }
    }
    
    // 루트부터 재귀적으로 Refit
    RefitRecursive(0);
}

void FBVH::Clear()
{
    Nodes.Empty();
    PrimitiveBounds.Empty();
    PrimitiveIndices.Empty();
    MaxDepth = 0;
}

/**
* @brief Ray와 BVH 충돌 검사하여 충돌한 PrimitiveCompnent 반환
*/
UPrimitiveComponent* FBVH::Intersect(const FVector& RayOrigin, const FVector& RayDirection, float& OutDistance) const
{
    if (Nodes.Num() == 0)
        return nullptr;

    TStatId BVHIntersectStatId;
    FScopeCycleCounter BVHIntersectTimer(BVHIntersectStatId);

    OutDistance = FLT_MAX;
    UPrimitiveComponent* HitPrimitive = nullptr;

    FOptimizedRay OptRay(RayOrigin, RayDirection);
    bool bHit = IntersectNode(0, OptRay, OutDistance, HitPrimitive);

    uint64_t IntersectCycles = BVHIntersectTimer.Finish();
    double IntersectTimeMs = FPlatformTime::ToMilliseconds(IntersectCycles);

    if (bHit)
    {
        char buf[256];
        sprintf_s(buf, "[BVH Pick] Hit primitive at distance %.3f (Time: %.3fms)\n",
            OutDistance, IntersectTimeMs);
        UE_LOG(buf);
        return HitPrimitive;
    }
    else
    {
        char buf[256];
        sprintf_s(buf, "[BVH Pick] No hit (Time: %.3fms)\n", IntersectTimeMs);
        UE_LOG(buf);
        return nullptr;
    }
}
/**
* @breif 쿼리 AABB와 교차하는 모든 Primitives를 찾습니다. (Broad Phase)
* @param QueryAABB 교차 검사를 수행할 AABB
* @param OutPrimitives 교차된 Primitives들의 목록 저장할 배열
*/
void FBVH::IntersectAABB(const FBound& QueryAABB, TArray<UPrimitiveComponent*>& OutPrimitives) const
{
    OutPrimitives.Empty();
    if (Nodes.Num() == 0)
    {
        return;
    }
    IntersectAABBRecursive(0, QueryAABB, OutPrimitives);
}
/**
* @brief 재귀적으로 Build하는 헬퍼
* @param FirstPrim 첫번째 Primitive
* @param PrimCount Primitive 개수
* @param Depth 현재 깊이
* @return 노드의 인덱스
*/
int FBVH::BuildRecursive(int FirstPrim, int PrimCount, int Depth)
{
    MaxDepth = FMath::Max(MaxDepth, Depth);

    int NodeIndex = Nodes.Num();
    FBVHNode NewNode;
    Nodes.Add(NewNode);
    FBVHNode& Node = Nodes[NodeIndex];

    // 현재 노드의 바운딩 박스 생성
    Node.BoundingBox = CalculateBounds(FirstPrim, PrimCount);

    // 재귀 종료 (리프노드의 최대치보다 작거나 최대 깊이를 넘은 경우)
    if (PrimCount <= MaxPrimsPerLeaf || Depth >= MaxBVHDepth)
    {
        Node.FirstPrim = FirstPrim;
        Node.PrimCount = PrimCount;
        return NodeIndex;
    }

    int BestAxis;
    float BestSplitPos;
    int SplitIndex = FindBestSplit(FirstPrim, PrimCount, BestAxis, BestSplitPos);

    if (SplitIndex == FirstPrim || SplitIndex == FirstPrim + PrimCount)
    {
        Node.FirstPrim = FirstPrim;
        Node.PrimCount = PrimCount;
        return NodeIndex;
    }

    int ActualSplit = PartitionPrimitives(FirstPrim, PrimCount, BestAxis, BestSplitPos);

    int LeftCount = ActualSplit - FirstPrim;
    int RightCount = PrimCount - LeftCount;

    if (LeftCount == 0 || RightCount == 0)
    {
        Node.FirstPrim = FirstPrim;
        Node.PrimCount = PrimCount;
        return NodeIndex;
    }

    Node.LeftChild = BuildRecursive(FirstPrim, LeftCount, Depth + 1);
    Node.RightChild = BuildRecursive(ActualSplit, RightCount, Depth + 1);

    return NodeIndex;
}
/**
  * @brief Refit을 위한 재귀 헬퍼. 자식 노드의 AABB를 합쳐 부모 노드의 AABB를 만듭니다.
*/
FBound FBVH::RefitRecursive(int NodeIndex)
{
    FBVHNode& Node = Nodes[NodeIndex];
    
    if (Node.IsLeaf())
    {
        // Leaf일 때, 포함된 Node들의 AABB를 합쳐 자신의 AABB 결정
        Node.BoundingBox = CalculateBounds(Node.FirstPrim, Node.PrimCount);
    }
    else 
    {
        // Internal일 때, 자식 노드 재귀 호출 후 합치기
        FBound LeftBounds = RefitRecursive(Node.LeftChild);
        FBound RightBounds = RefitRecursive(Node.RightChild);
        Node.BoundingBox = Union(LeftBounds, RightBounds);
    }
    return Node.BoundingBox;
}

float FBVH::SurfaceArea(const FBound& b) {
    FVector s = b.Max - b.Min;
    if (s.X <= 0 || s.Y <= 0 || s.Z <= 0) return 0.0f;
    return 2.0f * (s.X * s.Y + s.Y * s.Z + s.Z * s.X);
}

FBound FBVH::CalculateBounds(int FirstPrim, int PrimCount) const
{
    FBound Bounds;
    bool bFirst = true;

    for (int i = 0; i < PrimCount; ++i)
    {
        int PrimIndex = PrimitiveIndices[FirstPrim + i];
        const FBound& PrimBound = PrimitiveBounds[PrimIndex].Bounds;

        if (bFirst)
        {
            Bounds = PrimBound;
            bFirst = false;
        }
        else
        {
            Bounds.Min.X = FMath::Min(Bounds.Min.X, PrimBound.Min.X);
            Bounds.Min.Y = FMath::Min(Bounds.Min.Y, PrimBound.Min.Y);
            Bounds.Min.Z = FMath::Min(Bounds.Min.Z, PrimBound.Min.Z);

            Bounds.Max.X = FMath::Max(Bounds.Max.X, PrimBound.Max.X);
            Bounds.Max.Y = FMath::Max(Bounds.Max.Y, PrimBound.Max.Y);
            Bounds.Max.Z = FMath::Max(Bounds.Max.Z, PrimBound.Max.Z);
        }
    }

    return Bounds;
}

FBound FBVH::CalculateCentroidBounds(int FirstPrim, int PrimCount) const
{
    FBound Bounds;
    bool bFirst = true;

    for (int i = 0; i < PrimCount; ++i)
    {
        int PrimIndex = PrimitiveIndices[FirstPrim + i];
        const FVector& Center = PrimitiveBounds[PrimIndex].Center;

        if (bFirst)
        {
            Bounds.Min = Bounds.Max = Center;
            bFirst = false;
        }
        else
        {
            Bounds.Min.X = FMath::Min(Bounds.Min.X, Center.X);
            Bounds.Min.Y = FMath::Min(Bounds.Min.Y, Center.Y);
            Bounds.Min.Z = FMath::Min(Bounds.Min.Z, Center.Z);

            Bounds.Max.X = FMath::Max(Bounds.Max.X, Center.X);
            Bounds.Max.Y = FMath::Max(Bounds.Max.Y, Center.Y);
            Bounds.Max.Z = FMath::Max(Bounds.Max.Z, Center.Z);
        }
    }

    return Bounds;
}


int FBVH::FindBestSplit(int FirstPrim, int PrimCount, int& OutAxis, float& OutSplitPos)
{
    FBound CentroidBounds = CalculateCentroidBounds(FirstPrim, PrimCount);
    FBound ParentBounds = CalculateBounds(FirstPrim, PrimCount);

    FVector Extent = CentroidBounds.Max - CentroidBounds.Min;
    OutAxis = 0;
    if (Extent.Y > Extent.X) OutAxis = 1;
    if (Extent.Z > Extent[OutAxis]) OutAxis = 2;

    if (Extent[OutAxis] < KINDA_SMALL_NUMBER)
    {
        OutSplitPos = CentroidBounds.Min[OutAxis];
        return FirstPrim + PrimCount / 2;
    }

    // 1) 정렬 - TArray의 Sort 사용
    // 임시 배열 생성 후 정렬
    TArray<int> TempIndices;
    TempIndices.Reserve(PrimCount);
    for (int i = 0; i < PrimCount; ++i)
    {
        TempIndices.Add(PrimitiveIndices[FirstPrim + i]);
    }

    TempIndices.Sort([&](int A, int B)
    {
        return PrimitiveBounds[A].Center[OutAxis] < PrimitiveBounds[B].Center[OutAxis];
    });
    // 정렬된 결과를 다시 복사
    for (int i = 0; i < PrimCount; ++i)
    {
        PrimitiveIndices[FirstPrim + i] = TempIndices[i];
    }
    // 2) Prefix/Suffix AABB 계산
    TArray<FBound> Prefix;
    TArray<FBound> Suffix;
    Prefix.SetNum(PrimCount);
    Suffix.SetNum(PrimCount);

    Prefix[0] = PrimitiveBounds[PrimitiveIndices[FirstPrim]].Bounds;
    for (int i = 1; i < PrimCount; i++)
        Prefix[i] = Union(Prefix[i - 1], PrimitiveBounds[PrimitiveIndices[FirstPrim + i]].Bounds);

    Suffix[PrimCount - 1] = PrimitiveBounds[PrimitiveIndices[FirstPrim + PrimCount - 1]].Bounds;
    for (int i = PrimCount - 2; i >= 0; i--)
        Suffix[i] = Union(Suffix[i + 1], PrimitiveBounds[PrimitiveIndices[FirstPrim + i]].Bounds);
    
    // 3) SAH 비용 평가
    float BestCost = FLT_MAX;
    int BestSplit = FirstPrim + PrimCount / 2;
    float SA_P = SurfaceArea(ParentBounds) + 1e-6f;

    for (int i = 0; i < PrimCount - 1; i++)
    {
        int LeftCount = i + 1;
        int RightCount = PrimCount - LeftCount;

        float SA_L = SurfaceArea(Prefix[i]);
        float SA_R = SurfaceArea(Suffix[i + 1]);

        float Cost = 1.0f + (SA_L / SA_P) * LeftCount + (SA_R / SA_P) * RightCount;

        if (Cost < BestCost)
        {
            BestCost = Cost;
            BestSplit = FirstPrim + LeftCount;
            OutSplitPos = PrimitiveBounds[PrimitiveIndices[BestSplit]].Center[OutAxis];
        }
    }

    return BestSplit;
}

float FBVH::CalculateSAH(int FirstPrim, int LeftCount, int RightCount, const FBound& Parent) const
{
    FBound LB = CalculateBounds(FirstPrim, LeftCount);
    FBound RB = CalculateBounds(FirstPrim + LeftCount, RightCount);

    float SA_P = SurfaceArea(Parent) + 1e-6f;
    float SA_L = SurfaceArea(LB);
    float SA_R = SurfaceArea(RB);

    constexpr float Ct = 1.0f;
    constexpr float Ci = 1.0f;
    return Ct + Ci * ((SA_L / SA_P) * LeftCount + (SA_R / SA_P) * RightCount);
}

int FBVH::PartitionPrimitives(int FirstPrim, int PrimCount, int Axis, float SplitPos)
{
    int Left = FirstPrim;
    int Right = FirstPrim + PrimCount - 1;

    while (Left <= Right)
    {
        while (Left <= Right)
        {
            int LeftPrimIndex = PrimitiveIndices[Left];
            const FVector& LeftCenter = PrimitiveBounds[LeftPrimIndex].Center;
            if (LeftCenter[Axis] >= SplitPos)
                break;
            Left++;
        }

        while (Left <= Right)
        {
            int RightPrimIndex = PrimitiveIndices[Right];
            const FVector& RightCenter = PrimitiveBounds[RightPrimIndex].Center;
            if (RightCenter[Axis] < SplitPos)
                break;
            Right--;
        }

        if (Left < Right)
        {
            int Temp = PrimitiveIndices[Left];
            PrimitiveIndices[Left] = PrimitiveIndices[Right];
            PrimitiveIndices[Right] = Temp;
            Left++;
            Right--;
        }
    }

    return Left;
}

/**
* @brief BVH 노드를 재귀적으로 타고 들어가면서 Ray와 실제 충돌한 Primitive 얻어오기
*/
bool FBVH::IntersectNode(int NodeIndex,
    const FOptimizedRay& Ray,
    float& InOutDistance,
    UPrimitiveComponent*& OutPrimitive) const
{
    const FBVHNode& Node = Nodes[NodeIndex];

    float tNear;
    if (!Ray.IntersectAABB(Node.BoundingBox, tNear))
        return false;

    if (tNear >= InOutDistance)
        return false;

    if (Node.IsLeaf())
    {
        bool bHit = false;
        float Closest = InOutDistance;
        UPrimitiveComponent* ClosestPrimitive = nullptr;

        for (int i = 0; i < Node.PrimCount; ++i)
        {
            int PrimIndex = PrimitiveIndices[Node.FirstPrim + i];
            UPrimitiveComponent* Prim = PrimitiveBounds[PrimIndex].Primitive;

            float Dist;
            if (IntersectPrimitive(Prim, Ray.Origin, Ray.Direction, Dist))
            {
                if (Dist < Closest)
                {
                    Closest = Dist;
                    ClosestPrimitive = Prim;
                    bHit = true;
                }
            }
        }

        if (bHit)
        {
            InOutDistance = Closest;
            OutPrimitive = ClosestPrimitive;
        }

        return bHit;
    }

    struct ChildHit
    {
        int Index;
        float tNear;
        bool bValid;
    };

    auto TestChild = [&](int ChildIdx) -> ChildHit
        {
            if (ChildIdx < 0) return { ChildIdx, FLT_MAX, false };
            float tN;
            if (Ray.IntersectAABB(Nodes[ChildIdx].BoundingBox, tN))
                return { ChildIdx, tN, true };
            return { ChildIdx, FLT_MAX, false };
        };

    ChildHit L = TestChild(Node.LeftChild);
    ChildHit R = TestChild(Node.RightChild);

    bool bHit = false;

    if (L.bValid && R.bValid)
    {
        const ChildHit First = (L.tNear < R.tNear) ? L : R;
        const ChildHit Second = (L.tNear < R.tNear) ? R : L;

        if (IntersectNode(First.Index, Ray, InOutDistance, OutPrimitive))
            bHit = true;

        if (InOutDistance > Second.tNear)
        {
            if (IntersectNode(Second.Index, Ray, InOutDistance, OutPrimitive))
                bHit = true;
        }
    }
    else if (L.bValid)
    {
        if (IntersectNode(L.Index, Ray, InOutDistance, OutPrimitive))
            bHit = true;
    }
    else if (R.bValid)
    {
        if (IntersectNode(R.Index, Ray, InOutDistance, OutPrimitive))
            bHit = true;
    }

    return bHit;
}
/**
* @brief BVH 노드를 재귀적으로 순회하며 AABB와 교차하는 Primitives를 찾습니다.
* @param NodeIndex 현재 탐색 중인 노드의 인덱스
* @param QueryAABB 검사할 AABB
* @param OutPrimitives 결과를 저장할 배열
*/
void FBVH::IntersectAABBRecursive(int NodeIndex, const FBound& QueryAABB, TArray<UPrimitiveComponent*>& OutPrimitives) const
{
    const FBVHNode& Node = Nodes[NodeIndex];

    // 노드의 AABB와 쿼리 AABB가 교차 안하면 Pruning
    if (!Node.BoundingBox.Intersects(QueryAABB))
    {
        return;
    }

    if (Node.IsLeaf())
    {
        for (int i = 0;i < Node.PrimCount; ++i)
        {
            int PrimIndexInBVH = PrimitiveIndices[Node.FirstPrim + i];
            const FPrimitiveBounds& PrimBound = PrimitiveBounds[PrimIndexInBVH];

            if (PrimBound.Bounds.Intersects(QueryAABB))
            {
                OutPrimitives.Add(PrimBound.Primitive);
            }
        }
    }
    else // Internal Node인 경우
    {
        if (Node.LeftChild != -1)
        {
            IntersectAABBRecursive(Node.LeftChild, QueryAABB, OutPrimitives);
        }
        if (Node.RightChild != -1)
        {
            IntersectAABBRecursive(Node.RightChild, QueryAABB, OutPrimitives);
        }
    }
}

bool FBVH::IntersectPrimitive(const UPrimitiveComponent* Primitive, const FVector& RayOrigin, const FVector& RayDirection,
    float& OutDistance) const
{
    // This needs a proper implementation based on primitive type
    // For now, we can just intersect with the AABB as a proxy
    return Primitive->GetWorldBound().RayIntersects(RayOrigin, RayDirection, OutDistance);
}

void FBVH::Render(URenderer* Renderer) const
{
    if (!Renderer || Nodes.Num() == 0)
    {
        return;
    }
    // 루트 노드(인덱스 0)부터 재귀 렌더링 시작
    RenderRecursive(Renderer, 0, 0);
}

void FBVH::RenderRecursive(URenderer* Renderer, int NodeIndex, int Depth) const
{
    if (NodeIndex < 0 || (size_t)NodeIndex >= Nodes.Num())
    {
        return;
    }

    const FBVHNode& Node = Nodes[NodeIndex];

    FVector4 Color;
    switch (Depth % 5)
    {
    case 0: Color = FVector4(1.f, 0.f, 0.f, 1.f); break; // Red
    case 1: Color = FVector4(0.f, 1.f, 0.f, 1.f); break; // Green
    case 2: Color = FVector4(0.f, 0.f, 1.f, 1.f); break; // Blue
    case 3: Color = FVector4(1.f, 1.f, 0.f, 1.f); break; // Yellow
    case 4: Color = FVector4(0.f, 1.f, 1.f, 1.f); break; // Cyan
    }
    // 현재 노드의 AABB를 그리기
    Renderer->DrawAABB(Node.BoundingBox.Min, Node.BoundingBox.Max, Color);

    if (!Node.IsLeaf())
    {
        // 내부 노드인 경우, 자식 노드들에 대해 재귀적으로 렌더링
        RenderRecursive(Renderer, Node.LeftChild, Depth + 1);
        RenderRecursive(Renderer, Node.RightChild, Depth + 1);
    }
}