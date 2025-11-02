#include "pch.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
#include "Component/Public/SceneComponent.h"

IMPLEMENT_CLASS(APlayerCharacter, APawn)

APlayerCharacter::APlayerCharacter()
{
	bCanEverTick = true;
	MovementSpeed = 100.0f;
}

APlayerCharacter::~APlayerCharacter()
{
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[PlayerCharacter] BeginPlay: %s", GetName().ToString().c_str());
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	FVector Forward(1.0f, 0.0f, 0.0f);
	FVector NewLocation = GetActorLocation() + (Forward * Value * MovementSpeed * DT);
	SetActorLocation(NewLocation);
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	FVector Right(0.0f, 1.0f, 0.0f);
	FVector NewLocation = GetActorLocation() + (Right * Value * MovementSpeed * DT);
	SetActorLocation(NewLocation);
}

