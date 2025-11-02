#include "pch.h"
#include "GameMode/Public/GameModeBase.h"
#include "Actor/Public/Actor.h"
#include "GamePlay/Public/PlayerController.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
#include "Level/Public/World.h"
#include "Core/Public/NewObject.h"

IMPLEMENT_CLASS(AGameModeBase, AActor)

AGameModeBase::AGameModeBase()
{
	bCanEverTick = true;

	// Set default pawn class to APlayerCharacter
	DefaultPawnClass = APlayerCharacter::StaticClass();
}

AGameModeBase::~AGameModeBase()
{
}

void AGameModeBase::InitGame()
{
	UE_LOG("[GameMode] InitGame called");

	// Initialize game-specific settings here
	// This is called before BeginPlay
}

void AGameModeBase::StartPlay()
{
	UE_LOG("[GameMode] StartPlay called");

	// Initialize player controller and spawn default pawn
	InitializePlayerController();
}

void AGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[GameMode] BeginPlay: %s", GetName().ToString().c_str());

	// Start the game
	StartPlay();
}

void AGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameModeBase::InitializePlayerController()
{
	if (!OwningWorld)
	{
		UE_LOG_ERROR("[GameMode] Cannot initialize player controller: No owning world");
		return;
	}

	// Spawn player controller
	APlayerController* NewController = SpawnPlayerController();
	if (!NewController)
	{
		UE_LOG_ERROR("[GameMode] Failed to spawn player controller");
		return;
	}

	PlayerController.Set(NewController);

	// Spawn default pawn for the player
	APawn* NewPawn = SpawnDefaultPawnFor(NewController);
	if (NewPawn)
	{
		// Make the controller possess the pawn
		NewController->Possess(NewPawn);

		// Also notify the pawn that it has been possessed
		NewPawn->PossessedBy(NewController);

		UE_LOG_SUCCESS("[GameMode] Player controller initialized and possessing pawn");
	}
	else
	{
		UE_LOG_WARNING("[GameMode] Failed to spawn default pawn for player controller");
	}
}

APlayerController* AGameModeBase::SpawnPlayerController()
{
	if (!OwningWorld)
	{
		UE_LOG_ERROR("[GameMode] Cannot spawn player controller: No owning world");
		return nullptr;
	}

	// Spawn the player controller actor
	APlayerController* NewController = Cast<APlayerController>(
		OwningWorld->SpawnActor(APlayerController::StaticClass())
	);

	if (NewController)
	{
		NewController->Initialize();
		UE_LOG("[GameMode] Player controller spawned: %s", NewController->GetName().ToString().c_str());
	}

	return NewController;
}

APawn* AGameModeBase::SpawnDefaultPawnFor(APlayerController* NewPlayer)
{
	if (!OwningWorld)
	{
		UE_LOG_ERROR("[GameMode] Cannot spawn pawn: No owning world");
		return nullptr;
	}

	if (!DefaultPawnClass)
	{
		UE_LOG_ERROR("[GameMode] Cannot spawn pawn: DefaultPawnClass is null");
		return nullptr;
	}

	// Spawn the pawn at origin (you can customize spawn location)
	APawn* NewPawn = Cast<APawn>(
		OwningWorld->SpawnActor(DefaultPawnClass)
	);

	if (NewPawn)
	{
		UE_LOG("[GameMode] Default pawn spawned: %s", NewPawn->GetName().ToString().c_str());
	}

	return NewPawn;
}

void AGameModeBase::SetDefaultPawnClass(UClass* InPawnClass)
{
	if (InPawnClass && InPawnClass->IsChildOf(APawn::StaticClass()))
	{
		DefaultPawnClass = InPawnClass;
		UE_LOG("[GameMode] Default pawn class set to: %s", InPawnClass->GetName().ToString().c_str());
	}
	else
	{
		UE_LOG_ERROR("[GameMode] Invalid pawn class - must inherit from APawn");
	}
}