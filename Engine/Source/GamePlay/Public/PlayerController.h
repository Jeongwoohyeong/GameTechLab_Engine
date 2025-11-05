#pragma once
#include "Global/WeakObjectPtr.h"

class APlayerCharacter;
class APawn;
class UPlayerInput;
class AActor;
class APlayerCameraManager;

class APlayerController : public AActor
{
    GENERATED_BODY()
    DECLARE_CLASS(APlayerController, AActor)
public:
    APlayerController();
    ~APlayerController();

    void Initialize();
    
    void Tick(float DeltaTime) override;

    void SetInput(UPlayerInput* PlayerInput);

    void Possess(AActor* TargetActor);
    void UnPossess();

    AActor* GetControlledActor() const { return ControlledActor.Get(); }
    APawn* GetControlledPawn() const;

    // Getter for PlayerInput (Shift + F1)
    UPlayerInput* GetPlayerInput() const { return PlayerInput; }

    // ========== Camera Manager ==========

    /**
     * @brief Set the player camera manager
     * @param InCameraManager The camera manager to use
     */
    void SetPlayerCameraManager(APlayerCameraManager* InCameraManager);

    /**
     * @brief Get the player camera manager
     * @return The player camera manager
     */
    APlayerCameraManager* GetPlayerCameraManager() const { return PlayerCameraManager; }

private:
    // Value값의 부호를 바꾸면 반대방향 이동
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);

private:
    TWeakObjectPtr<AActor> ControlledActor;
    UPlayerInput* PlayerInput = nullptr;
    APlayerCameraManager* PlayerCameraManager = nullptr;

    float MoveSpeed = 1.0f;
};
