#include "pch.h"
#include "ProjectileMovementComponent.h"

UProjectileMovementComponent::UProjectileMovementComponent() { SetTickEnabled(true); }

void UProjectileMovementComponent::Move(float DeltaSeconds)
{
	GetOwner()->AddActorLocalLocation(LaunchDirection * DeltaSeconds);
}

void UProjectileMovementComponent::TickComponent(float DeltaSeconds)
{
	Move(DeltaSeconds);
}

UObject* UProjectileMovementComponent::Duplicate()
{
	UProjectileMovementComponent* DuplicatedComponent = NewObject<UProjectileMovementComponent>(*this);

	// 복제된 컴포넌트는 초기화되지 않은 상태로 시작
	DuplicatedComponent->bIsInitialized = false;
	DuplicatedComponent->LaunchDirection = LaunchDirection;

	DuplicatedComponent->DuplicateSubObjects();
	return DuplicatedComponent;
}

void UProjectileMovementComponent::DuplicateSubObjects()
{
	Super_t::DuplicateSubObjects();
}

void UProjectileMovementComponent::Serialize(FObjectData* Data)
{
	FProjectileMovementComponentData* ComponentData = dynamic_cast<FProjectileMovementComponentData*>(Data);
	assert(ComponentData, "UProjectileMovementComponent::Serialize got wrong data type.");

	UActorComponent::Serialize(Data);
	ComponentData->LaunchDirection = LaunchDirection;
}

void UProjectileMovementComponent::DeSerialize(FObjectData* Data)
{
	UActorComponent::DeSerialize(Data);
}
