#pragma once
#include "Pawn/Public/Pawn.h"

class USceneComponent;

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

protected:
    // Default movement speed
    float MovementSpeed = 100.0f;
};
