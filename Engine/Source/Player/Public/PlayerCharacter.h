#pragma once
#include "Pawn/Public/Pawn.h"

class USphereComponent;
class UCameraComponent;
class UPointLightComponent;
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

    // UObject interface


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

    // Get camera component
    UCameraComponent* GetCameraComponent() const { return CameraComponent; }

protected:
    // Default movement speed
    float MovementSpeed = 10.0f;

    // Default rotation speed (degrees per second)
    float RotationSpeed = 50.0f;

    // Mouse sensitivity for pitch/yaw rotation
    float MouseSensitivity = 10.0f;

    // Track forward movement for engine lights
    bool bIsMovingForward = false;

    USphereComponent* CollisionComponent = nullptr;
    UStaticMeshComponent* StaticMeshComponent = nullptr;
    UCameraComponent* CameraComponent = nullptr;
    UPointLightComponent* PointLight1 = nullptr;
    UPointLightComponent* PointLight2 = nullptr;
};
