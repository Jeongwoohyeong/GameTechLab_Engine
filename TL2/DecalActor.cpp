#include "pch.h"
#include "DecalActor.h"
#include "AABoundingBoxComponent.h"
#include "ObjectFactory.h"

ADecalActor::ADecalActor() {}

void ADecalActor::Initialize()
{
	Name = "Decal Actor";
	DecalComponent = CreateDefaultSubobject<UDecalComponent>("DecalComponent");
	RootComponent = DecalComponent;
	AddComponent(DecalComponent);

	// Collision Component 추가 (피킹 및 선택을 위해)
	CollisionComponent = CreateDefaultSubobject<UAABoundingBoxComponent>(FName("CollisionBox"));
	CollisionComponent->SetupAttachment(RootComponent);

	// Decal 기본 크기 설정 (1x1x1)
	SetActorScale(FVector(1.0f, 1.0f, 1.0f));

	UE_LOG("[DecalActor] Constructor called (UUID: %u)\n", UUID);
}

void ADecalActor::Tick(float DeltaTime)
{
	AActor::Tick(DeltaTime);

	//// CollisionComponent 업데이트 (항상 업데이트하여 피킹 가능하도록)
	//if (CollisionComponent && DecalComponent)
	//{
	//	// Unit Cube의 8개 정점으로 바운딩 박스 설정
	//	TArray<FNormalVertex> CubeVertices = {
	//		FNormalVertex(FVector(-0.5f, -0.5f, -0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(0.5f, -0.5f, -0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(0.5f,  0.5f, -0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(-0.5f,  0.5f, -0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(-0.5f, -0.5f,  0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(0.5f, -0.5f,  0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(0.5f,  0.5f,  0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0)),
	//		FNormalVertex(FVector(-0.5f,  0.5f,  0.5f), FVector(0, 0, 0), FVector4(1, 1, 1, 1), FVector2D(0, 0))
	//	};
	//	CollisionComponent->SetFromVertices(CubeVertices);
	//}	
}

ADecalActor::~ADecalActor()
{
	if (DecalComponent)
	{
		ObjectFactory::DeleteObject(DecalComponent);
	}
	DecalComponent = nullptr;
}

void ADecalActor::SetDecalComponent(UDecalComponent* InDecalComponent)
{
	DecalComponent = InDecalComponent;
}

UObject* ADecalActor::Duplicate()
{
	UE_LOG("[DecalActor] Duplicate() called on UUID %u at location (%.2f, %.2f, %.2f)\n",
		UUID, GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);

	// 원본(this)의 컴포넌트 저장
	USceneComponent* OriginalRoot = this->RootComponent;

	// 얕은 복사 수행
	ADecalActor* DuplicatedActor = NewObject<ADecalActor>(*this);

	// 원본의 RootComponent(DecalComponent) 복제
	if (OriginalRoot)
	{
		DuplicatedActor->RootComponent = Cast<USceneComponent>(OriginalRoot->Duplicate());
	}

	// OwnedComponents 재구성 및 타입별 포인터 재설정
	DuplicatedActor->DuplicateSubObjects();
	
	return DuplicatedActor;
}

void ADecalActor::DuplicateSubObjects()
{
	// Duplicate()에서 이미 RootComponent를 복제했으므로
	// 부모 클래스가 OwnedComponents를 재구성
	Super_t::DuplicateSubObjects();

	// 타입별 포인터 재설정
	DecalComponent = Cast<UDecalComponent>(RootComponent);

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

bool ADecalActor::DeleteComponent(USceneComponent* ComponentToDelete)
{
	// CollisionComponent 삭제 처리
	if (ComponentToDelete == CollisionComponent)
	{
		CollisionComponent = nullptr;
	}
	// DecalComponent는 Root이므로 삭제 불가
	else if (ComponentToDelete == DecalComponent)
	{
		UE_LOG("루트 컴포넌트(DecalComponent)는 직접 삭제할 수 없습니다.");
		return false;
	}

	// 부모 클래스의 원래 기능 호출
	return AActor::DeleteComponent(ComponentToDelete);
}