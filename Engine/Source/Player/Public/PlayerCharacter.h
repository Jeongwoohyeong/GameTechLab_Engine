#pragma once
#include "Pawn/Public/Pawn.h"

class UCapsuleComponent;
class UBoxComponent;
struct FHitResult;

/**
 * @brief APlayerCharacter is the default pawn class for player-controlled characters.
 * Inherits from APawn and adds player-specific functionality.
 */
UCLASS()
class APlayerCharacter : public APawn
{
    GENERATED_BODY()
    DECLARE_CLASS(APlayerCharacter, APawn)

public:
    APlayerCharacter();
    virtual ~APlayerCharacter();

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Input handling (override from APawn)
    virtual void MoveForward(float Value) override;
    virtual void MoveRight(float Value) override;
    virtual void Turn(float Value) override;
    virtual void LookUp(float Value) override;

    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
    
    void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    void OnHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& OutHit);

    // Camera shake
    void StartCameraShake(float Intensity = 1.0f, float Duration = 0.5f);

protected:
    // Default movement speed
    float MovementSpeed = 100.0f;

    // Default rotation speed (degrees per second)
    float RotationSpeed = 90.0f;

    // Mouse sensitivity for pitch/yaw rotation
    float MouseSensitivity = 120.0f;

    UCapsuleComponent* CollisionComponent = nullptr;
    UBoxComponent* WingCollision = nullptr;
    UStaticMeshComponent* StaticMeshComponent = nullptr;

    // Camera shake variables
    bool bIsCameraShaking = false;
    float CameraShakeTimer = 0.0f;
    float CameraShakeDuration = 0.5f;
    float CameraShakeIntensity = 1.0f;
    FVector OriginalCameraOffset = FVector(0.0f, 0.0f, 0.0f);
};
