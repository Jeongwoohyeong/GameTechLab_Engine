#include "pch.h"
#include "AABoundingBoxComponent.h"
#include "StaticMeshActor.h"
#include "ObjectFactory.h"

AStaticMeshActor::AStaticMeshActor() {}

void AStaticMeshActor::Initialize()
{
    Name = "Static Mesh Actor";
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
    RootComponent = StaticMeshComponent;
    AddComponent(StaticMeshComponent);

    CollisionComponent = CreateDefaultSubobject<UAABoundingBoxComponent>(FName("CollisionBox"));
    CollisionComponent->SetupAttachment(RootComponent);
}

void AStaticMeshActor::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
}

AStaticMeshActor::~AStaticMeshActor()
{
    if (StaticMeshComponent)
    {
        ObjectFactory::DeleteObject(StaticMeshComponent);
    }
    StaticMeshComponent = nullptr;
}

void AStaticMeshActor::SetStaticMeshComponent(UStaticMeshComponent* InStaticMeshComponent)
{
    StaticMeshComponent = InStaticMeshComponent;
}

void AStaticMeshActor::SetCollisionComponent(EPrimitiveType InType)
{
    if (!CollisionComponent) {
        return;
    }
    CollisionComponent->SetFromVertices(StaticMeshComponent->GetStaticMesh()->GetStaticMeshAsset()->Vertices);
    CollisionComponent->SetPrimitiveType(InType);
}

UObject* AStaticMeshActor::Duplicate()
{
    // 원본(this)의 컴포넌트들 저장
    USceneComponent* OriginalRoot = this->RootComponent;

    // 얕은 복사 수행
    AStaticMeshActor* DuplicatedActor = NewObject<AStaticMeshActor>(*this);

    // 원본의 RootComponent(StaticMeshComponent) 복제
    if (OriginalRoot)
    {
        DuplicatedActor->RootComponent = Cast<USceneComponent>(OriginalRoot->Duplicate());
    }

    // OwnedComponents 재구성 및 타입별 포인터 재설정
    DuplicatedActor->DuplicateSubObjects();

    return DuplicatedActor;
}

void AStaticMeshActor::DuplicateSubObjects()
{
    // Duplicate()에서 이미 RootComponent를 복제했으므로
    // 부모 클래스가 OwnedComponents를 재구성
    Super_t::DuplicateSubObjects();

    // 타입별 포인터 재설정
    StaticMeshComponent = Cast<UStaticMeshComponent>(RootComponent);

    // CollisionComponent 찾기
    for (UActorComponent* Comp : OwnedComponents)
    {
        if (UAABoundingBoxComponent* BBoxComp = Cast<UAABoundingBoxComponent>(Comp))
        {
            CollisionComponent = BBoxComp;
            break;
        }
    }
}

// 특화된 멤버 컴포넌트 CollisionComponent, StaticMeshComponent 는 삭제 시 포인터를 초기화합니다.
bool AStaticMeshActor::DeleteComponent(USceneComponent* ComponentToDelete)
{
    // 1. [자식 클래스의 추가 처리] 삭제 대상이 나의 특정 컴포넌트인지 확인합니다.
    if (ComponentToDelete == CollisionComponent)
    {
        // 맞다면, 나의 멤버 포인터를 nullptr로 설정합니다.
        CollisionComponent = nullptr;
    }
    else if (ComponentToDelete == StaticMeshComponent)
    {
        // AStaticMeshActor는 StaticMeshComponent가 Root 이기 때문에 삭제할 수 없음
        UE_LOG("루트 컴포넌트는 직접 삭제할 수 없습니다.");
        return false;
    }

    // 2. [부모 클래스의 원래 기능 호출]
    // 기본적인 삭제 로직(소유 목록 제거, 메모리 해제 등)은 부모에게 위임합니다.
    // Super:: 키워드를 사용합니다.
    return AActor::DeleteComponent(ComponentToDelete);
}
