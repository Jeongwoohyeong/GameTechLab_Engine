#include "pch.h"
#include "HeightFog.h"
#include "HeightFogComponent.h"
#include "ObjectFactory.h"

//IMPLEMENT_CLASS(AHeightFog)

void AHeightFog::Initialize()
{
	Name = "Height Fog";
	HeightFogComponent = CreateDefaultSubobject<UHeightFogComponent>("HeightFogComponent");
	RootComponent = HeightFogComponent;
	AddSceneComponent(HeightFogComponent);
}

AHeightFog::~AHeightFog()
{
	if (HeightFogComponent)
	{
		ObjectFactory::DeleteObject(HeightFogComponent);
	}
	HeightFogComponent = nullptr;
}

UObject* AHeightFog::Duplicate()
{
	UE_LOG("[AHeightFog] Duplicate() called on UUID %u at location (%.2f, %.2f, %.2f)\n",
		UUID, GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);

	// 원본(this)의 컴포넌트 저장
	USceneComponent* OriginalRoot = this->RootComponent;

	// 얕은 복사 수행
	AHeightFog* DuplicatedActor = NewObject<AHeightFog>(*this);

	// 원본의 RootComponent(HeightFogComponent) 복제
	if (OriginalRoot)
	{
		DuplicatedActor->RootComponent = Cast<USceneComponent>(OriginalRoot->Duplicate());
	}

	// OwnedComponents 재구성 및 타입별 포인터 재설정
	DuplicatedActor->DuplicateSubObjects();

	return DuplicatedActor;
}

void AHeightFog::DuplicateSubObjects()
{
	// Duplicate()에서 이미 RootComponent를 복제했으므로
	// 부모 클래스가 OwnedComponents를 재구성
	Super_t::DuplicateSubObjects();

	// 타입별 포인터 재설정
	HeightFogComponent = Cast<UHeightFogComponent>(RootComponent);
}
