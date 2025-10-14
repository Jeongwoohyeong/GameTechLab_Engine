#include "pch.h"
#include "RotationMovementComponent.h"

URotationMovementComponent::URotationMovementComponent() { SetTickEnabled(true); }

void URotationMovementComponent::Move(float DeltaSeconds)
{
	const FQuat Quat = FQuat::MakeFromEuler(RotationAngle * DeltaSeconds);
	GetOwner()->AddActorLocalRotation(Quat);
}

void URotationMovementComponent::TickComponent(float DeltaSeconds)
{
	Move(DeltaSeconds);
}

UObject* URotationMovementComponent::Duplicate()
{
	URotationMovementComponent* DuplicatedComponent = NewObject<URotationMovementComponent>(*this);

	// 복제된 컴포넌트는 초기화되지 않은 상태로 시작
	DuplicatedComponent->bIsInitialized = false;
	DuplicatedComponent->RotationAngle = RotationAngle;

	DuplicatedComponent->DuplicateSubObjects();
	return DuplicatedComponent;
}

void URotationMovementComponent::DuplicateSubObjects()
{
	Super_t::DuplicateSubObjects();
}

void URotationMovementComponent::Serialize(FObjectData* Data)
{
	FRotationMovementComponentData* ComponentData = dynamic_cast<FRotationMovementComponentData*>(Data);
	assert(ComponentData, "URotationMovementComponent::Serialize got wrong data type.");

	UActorComponent::Serialize(Data);
	ComponentData->RotationAngle = RotationAngle;
}

void URotationMovementComponent::DeSerialize(FObjectData* Data)
{
	FRotationMovementComponentData* ComponentData = dynamic_cast<FRotationMovementComponentData*>(Data);
	assert(ComponentData, "URotationMovementComponent::DeSerialize got wrong data type.");

	UActorComponent::DeSerialize(Data);
	RotationAngle = ComponentData->RotationAngle;
}