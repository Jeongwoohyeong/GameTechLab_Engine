#include "pch.h"
#include "GameMode/Public/GameModeBase.h"
#include "Actor/Public/Actor.h"
#include "GamePlay/Public/PlayerController.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
#include "Level/Public/World.h"
#include "Level/Public/Level.h"
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

	UE_LOG("[GameMode] OwningWorld type: %d (0=Editor, 1=Game, 2=PIE)", static_cast<int>(OwningWorld->GetWorldType()));
	UE_LOG("[GameMode] OwningWorld pointer: %p", OwningWorld);

	if (!DefaultPawnClass)
	{
		UE_LOG_ERROR("[GameMode] Cannot spawn pawn: DefaultPawnClass is null");
		return nullptr;
	}

	// Spawn the pawn at a visible location (in front of camera)
	APawn* NewPawn = Cast<APlayerCharacter>(
		OwningWorld->SpawnActor(DefaultPawnClass)
	);

	if (NewPawn)
	{
		// Set to a visible location (0, 0, 0) - origin
		FVector SpawnLocation(0.0f, 0.0f, 0.0f);
		NewPawn->SetActorLocation(SpawnLocation);

		UE_LOG("[GameMode] Default pawn spawned: %s at location (%.1f, %.1f, %.1f)",
			NewPawn->GetName().ToString().c_str(),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

		UWorld* PawnWorld = NewPawn->GetTypedOuter<UWorld>();
		UE_LOG("[GameMode] Pawn's outer World: %p (type=%d)",
			PawnWorld,
			PawnWorld ? static_cast<int>(PawnWorld->GetWorldType()) : -1);

		// PIE World의 전체 Actor 리스트 확인
		UE_LOG("[GameMode] PIE World total actors: %d", OwningWorld->GetLevel()->GetLevelActors().size());
		for (auto* Actor : OwningWorld->GetLevel()->GetLevelActors())
		{
			if (Actor)
			{
				UE_LOG("[GameMode]   - Actor: %s at (%.1f, %.1f, %.1f)",
					Actor->GetClass()->GetName().ToString().c_str(),
					Actor->GetActorLocation().X,
					Actor->GetActorLocation().Y,
					Actor->GetActorLocation().Z);
			}
		}

		// Component 확인
		int32 ComponentCount = NewPawn->GetOwnedComponents().size();
		UE_LOG("[GameMode] Pawn has %d components", ComponentCount);

		for (auto* Comp : NewPawn->GetOwnedComponents())
		{
			if (Comp)
			{
				UE_LOG("[GameMode]   - Component: %s at WorldLocation=(%.1f, %.1f, %.1f)",
					Comp->GetClass()->GetName().ToString().c_str(),
					Cast<USceneComponent>(Comp) ? Cast<USceneComponent>(Comp)->GetWorldLocation().X : 0.0f,
					Cast<USceneComponent>(Comp) ? Cast<USceneComponent>(Comp)->GetWorldLocation().Y : 0.0f,
					Cast<USceneComponent>(Comp) ? Cast<USceneComponent>(Comp)->GetWorldLocation().Z : 0.0f);
			}
		}
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