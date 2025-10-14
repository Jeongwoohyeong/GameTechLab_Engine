#include "pch.h"
#include "SpotlightActor.h"
#include "SpotlightComponent.h"

ASpotlightActor::ASpotlightActor() {}

void ASpotlightActor::Initialize()
{
	Name = "Spotlight Actor";
	SpotlightComponent = CreateDefaultSubobject<USpotlightComponent>("SpotlightComponent");
	RootComponent = SpotlightComponent;
	AddSceneComponent(SpotlightComponent);

	// Billboard Component (표시를 위해)
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(FName("BillboardComponent"));
	BillboardComponent->SetupAttachment(RootComponent);
	BillboardComponent->SetTexture("Editor/Icon/SpotLight_64x.dds");
	BillboardComponent->SetRelativeLocation(FVector(0, 0, 0));
	BillboardComponent->SetDisplayedOnPlayMode(false); // 플레이 모드에서는 숨김

	// Spotlight 기본 크기 설정
	SetActorScale(FVector(10.0f, 10.0f, 10.0f));
}

ASpotlightActor::~ASpotlightActor()
{
	if (SpotlightComponent)
	{
		ObjectFactory::DeleteObject(SpotlightComponent);
	}

	SpotlightComponent = nullptr;
}

bool ASpotlightActor::DeleteSceneComponent(USceneComponent* ComponentToDelete)
{
	// BillboardComponent 삭제 처리
	if (ComponentToDelete == BillboardComponent)
	{
		BillboardComponent = nullptr;
	}
	// SpotlightComponent는 Root이므로 삭제 불가
	else if (ComponentToDelete == SpotlightComponent)
	{
		UE_LOG("루트 컴포넌트(SpotlightComponent)는 직접 삭제할 수 없습니다.");
		return false;
	}

	// 부모 클래스의 원래 기능 호출
	return AActor::DeleteSceneComponent(ComponentToDelete);
}

void ASpotlightActor::SetSpotlightComponent(USpotlightComponent* InSpotlightComponent)
{
	SpotlightComponent = InSpotlightComponent;
}

UObject* ASpotlightActor::Duplicate()
{
	UE_LOG("[SpotlightActor] Duplicate() called on UUID %u at location (%.2f, %.2f, %.2f)\n",
		UUID, GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);

	// 원본(this)의 컴포넌트 저장
	USceneComponent* OriginalRoot = this->RootComponent;

	// 얕은 복사 수행 (생성자 실행됨 - SpotlightComponent 생성)
	ASpotlightActor* DuplicatedActor = NewObject<ASpotlightActor>(*this);

	// 생성자가 만든 컴포넌트 삭제
	if (DuplicatedActor->SpotlightComponent)
	{
		DuplicatedActor->OwnedSceneComponents.Remove(DuplicatedActor->SpotlightComponent);
		ObjectFactory::DeleteObject(DuplicatedActor->SpotlightComponent);
		DuplicatedActor->SpotlightComponent = nullptr;
	}

	if (DuplicatedActor->BillboardComponent)
	{
		DuplicatedActor->OwnedSceneComponents.Remove(DuplicatedActor->BillboardComponent);
		ObjectFactory::DeleteObject(DuplicatedActor->BillboardComponent);
		DuplicatedActor->BillboardComponent = nullptr;
	}

	DuplicatedActor->RootComponent = nullptr;
	DuplicatedActor->OwnedSceneComponents.clear();

	// 원본의 RootComponent(SpotlightComponent) 복제
	if (OriginalRoot)
	{
		DuplicatedActor->RootComponent = Cast<USceneComponent>(OriginalRoot->Duplicate());
	}

	// OwnedComponents 재구성 및 타입별 포인터 재설정
	DuplicatedActor->DuplicateSubObjects();

	return DuplicatedActor;
}

void ASpotlightActor::DuplicateSubObjects()
{
	// Duplicate()에서 이미 RootComponent를 복제했으므로
	// 부모 클래스가 OwnedComponents를 재구성
	Super_t::DuplicateSubObjects();

	// 타입별 포인터 재설정
	SpotlightComponent = Cast<USpotlightComponent>(RootComponent);

	// BillboardComponent 찾기
	for (UActorComponent* Comp : OwnedSceneComponents)
	{
		if (UBillboardComponent* BillboardComp = Cast<UBillboardComponent>(Comp))
		{
			BillboardComponent = BillboardComp;
			break;
		}
	}
}
