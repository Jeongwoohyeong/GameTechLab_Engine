#include "pch.h"
#include "GameMode/Public/GameModeBase.h"
#include "Actor/Public/Actor.h"
#include "GamePlay/Public/PlayerController.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
#include "Level/Public/World.h"
#include "Level/Public/Level.h"
#include "Core/Public/NewObject.h"
#include "Manager/UI/Public/ViewportManager.h"
#include "Render/UI/Viewport/Public/Viewport.h"
#include "Render/UI/Viewport/Public/ViewportClient.h"
#include "Editor/Public/Camera.h"
#include "Editor/Public/Editor.h"

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
	UE_LOG("[GameModeBase] InitGame called");

	// Initialize game-specific settings here
	// This is called before BeginPlay
}

void AGameModeBase::StartPlay()
{
	UE_LOG("[GameModeBase] StartPlay called");

	// Initialize player controller and spawn default pawn
	// InitializePlayerController();
}

void AGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[GameModeBase] BeginPlay: %s", GetName().ToString().c_str());

	// Start the game
	StartPlay();
}

void AGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameModeBase::InitializeLuaScript()
{
}

void AGameModeBase::InitializePlayerController()
{
	if (!OwningWorld)
	{
		UE_LOG_ERROR("[GameModeBase] Cannot initialize player controller: No owning world");
		return;
	}

	// Spawn player controller
	APlayerController* NewController = SpawnPlayerController();
	if (!NewController)
	{
		UE_LOG_ERROR("[GameModeBase] Failed to spawn player controller");
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

		// 불법증축: PIE 카메라를 플레이어에 붙이기
		SetupPIECamera(NewPawn);

		UE_LOG_SUCCESS("[GameModeBase] Player controller initialized and possessing pawn");
	}
	else
	{
		UE_LOG_WARNING("[GameModeBase] Failed to spawn default pawn for player controller");
	}
}

APlayerController* AGameModeBase::SpawnPlayerController()
{
	if (!OwningWorld)
	{
		UE_LOG_ERROR("[GameModeBase] Cannot spawn player controller: No owning world");
		return nullptr;
	}

	// Spawn the player controller actor
	APlayerController* NewController = Cast<APlayerController>(
		OwningWorld->SpawnActor(APlayerController::StaticClass())
	);

	if (NewController)
	{
		NewController->Initialize();
		UE_LOG("[GameModeBase] Player controller spawned: %s", NewController->GetName().ToString().c_str());
	}

	return NewController;
}

APawn* AGameModeBase::SpawnDefaultPawnFor(APlayerController* NewPlayer)
{
	if (!OwningWorld)
	{
		UE_LOG_ERROR("[GameModeBase] Cannot spawn pawn: No owning world");
		return nullptr;
	}

	UE_LOG("[GameModeBase] OwningWorld type: %d (0=Editor, 1=Game, 2=PIE)", static_cast<int>(OwningWorld->GetWorldType()));
	UE_LOG("[GameModeBase] OwningWorld pointer: %p", OwningWorld);

	if (!DefaultPawnClass)
	{
		UE_LOG_ERROR("[GameModeBase] Cannot spawn pawn: DefaultPawnClass is null");
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
		NewPawn->SetActorScale3D(FVector(5.0f, 5.0f, 5.0f));

		UE_LOG("[GameModeBase] Default pawn spawned: %s at location (%.1f, %.1f, %.1f)",
			NewPawn->GetName().ToString().c_str(),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

		UWorld* PawnWorld = NewPawn->GetTypedOuter<UWorld>();
		UE_LOG("[GameModeBase] Pawn's outer World: %p (type=%d)",
			PawnWorld,
			PawnWorld ? static_cast<int>(PawnWorld->GetWorldType()) : -1);

		// PIE World의 전체 Actor 리스트 확인
		UE_LOG("[GameModeBase] PIE World total actors: %d", OwningWorld->GetLevel()->GetLevelActors().size());
		for (auto* Actor : OwningWorld->GetLevel()->GetLevelActors())
		{
			if (Actor)
			{
				UE_LOG("[GameModeBase]   - Actor: %s at (%.1f, %.1f, %.1f)",
					Actor->GetClass()->GetName().ToString().c_str(),
					Actor->GetActorLocation().X,
					Actor->GetActorLocation().Y,
					Actor->GetActorLocation().Z);
			}
		}

		// Component 확인
		int32 ComponentCount = NewPawn->GetOwnedComponents().size();
		UE_LOG("[GameModeBase] Pawn has %d components", ComponentCount);

		for (auto* Comp : NewPawn->GetOwnedComponents())
		{
			if (Comp)
			{
				UE_LOG("[GameModeBase]   - Component: %s at WorldLocation=(%.1f, %.1f, %.1f)",
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
		UE_LOG("[GameModeBase] Default pawn class set to: %s", InPawnClass->GetName().ToString().c_str());
	}
	else
	{
		UE_LOG_ERROR("[GameModeBase] Invalid pawn class - must inherit from APawn");
	}
}

void AGameModeBase::SetupPIECamera(APawn* InPawn)
{
	if (!InPawn)
	{
		return;
	}

	// ViewportManager에서 PIE 활성 뷰포트 가져오기
	UViewportManager& ViewportManager = UViewportManager::GetInstance();
	int32 PIEViewportIndex = ViewportManager.GetPIEActiveViewportIndex();

	if (PIEViewportIndex < 0 || PIEViewportIndex >= ViewportManager.GetViewports().size())
	{
		UE_LOG_ERROR("[GameMode] Invalid PIE viewport index: %d", PIEViewportIndex);
		return;
	}

	FViewport* PIEViewport = ViewportManager.GetViewports()[PIEViewportIndex];
	if (!PIEViewport)
	{
		UE_LOG_ERROR("[GameMode] PIE viewport is null");
		return;
	}

	FViewportClient* ViewportClient = PIEViewport->GetViewportClient();
	if (!ViewportClient)
	{
		UE_LOG_ERROR("[GameMode] ViewportClient is null");
		return;
	}

	UCamera* PIECamera = ViewportClient->GetCamera();
	if (!PIECamera)
	{
		UE_LOG_ERROR("[GameMode] PIE camera is null");
		return;
	}

	// 카메라를 플레이어 뒤쪽 상단에 배치 (오프셋: 뒤 -50, 위 50)
	FVector CameraOffset(-30.0f, 0.0f, 15.0f);
	PIECamera->SetFollowTarget(InPawn, CameraOffset);

	// PIE 전용: FOV 120도로 설정 (비행기 전체 보이게)
	PIECamera->SetFovY(120.0f);

	UE_LOG_SUCCESS("[GameMode] PIE camera attached to player with offset (%.1f, %.1f, %.1f), FOV=120",
		CameraOffset.X, CameraOffset.Y, CameraOffset.Z);
}