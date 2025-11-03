#include "pch.h"
#include "Pawn/Public/Pawn.h"
#include "GamePlay/Public/PlayerController.h"

IMPLEMENT_CLASS(APawn, AActor)

APawn::APawn()
{
	bCanEverTick = true;
}

APawn::~APawn()
{
}

void APawn::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[Pawn] BeginPlay: %s", GetName().ToString().c_str());
}

void APawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APawn::PossessedBy(APlayerController* NewController)
{
	if (!NewController)
	{
		UE_LOG_WARNING("[Pawn] Attempted to possess with null controller");
		return;
	}

	// Unpossess current controller if any
	if (Controller.IsValid())
	{
		UnPossessed();
	}

	Controller.Set(NewController);
	UE_LOG("[Pawn] %s possessed by Controller", GetName().ToString().c_str());
}

void APawn::UnPossessed()
{
	if (Controller.IsValid())
	{
		UE_LOG("[Pawn] %s unpossessed", GetName().ToString().c_str());
		Controller.Reset();
	}
}

APlayerController* APawn::GetController() const
{
	return Controller.Get();
}

