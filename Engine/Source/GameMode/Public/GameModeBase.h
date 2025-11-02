#pragma once
#include "Actor/Public/Actor.h"
#include "Global/WeakObjectPtr.h"
#include "Core/Public/Class.h"

class APlayerController;
class APawn;
class UWorld;

/**
 * @brief AGameModeBase is the base class for game modes.
 * It defines the game rules and manages the spawning of player controllers and default pawns.
 * Similar to Unreal Engine's AGameModeBase.
 */
UCLASS()
class AGameModeBase : public AActor
{
    GENERATED_BODY()
    DECLARE_CLASS(AGameModeBase, AActor)

public:
    AGameModeBase();
    virtual ~AGameModeBase();

    // Lifecycle
    virtual void InitGame();
    virtual void StartPlay();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Player Management
    virtual void InitializePlayerController();
    virtual APlayerController* SpawnPlayerController();
    virtual APawn* SpawnDefaultPawnFor(APlayerController* NewPlayer);

    // Getters
    APlayerController* GetPlayerController() const { return PlayerController.Get(); }
    UClass* GetDefaultPawnClass() const { return DefaultPawnClass; }

    // Setters
    void SetDefaultPawnClass(UClass* InPawnClass);

protected:
    // PIE Camera Setup (불법증축!)
    void SetupPIECamera(APawn* InPawn);

    // Default pawn class to spawn for players
    UClass* DefaultPawnClass = nullptr;

    // Reference to the player controller
    TWeakObjectPtr<APlayerController> PlayerController;

    // World reference (set by World when GameMode is created)
    UWorld* OwningWorld = nullptr;

public:
    void SetOwningWorld(UWorld* InWorld) { OwningWorld = InWorld; }
    UWorld* GetWorld() const { return OwningWorld; }
};
