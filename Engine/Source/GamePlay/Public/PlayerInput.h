#pragma once
#include "Global/DelegateMacros.h"


DECLARE_DYNAMIC_DELEGATE(FOnMoveForward, float);
DECLARE_DYNAMIC_DELEGATE(FOnMoveRight, float);
DECLARE_DYNAMIC_DELEGATE(FOnTurn, float);
DECLARE_DYNAMIC_DELEGATE(FOnLookUp, float);

class UPlayerInput : public UObject
{
    GENERATED_BODY()
    DECLARE_CLASS(UPlayerInput, UObject)
public:
    UPlayerInput();
    ~UPlayerInput();

    void Update(float DeltaTime);

    // Enable/Disable input (Shift + F1)
    void SetInputEnabled(bool bEnabled) { bInputEnabled = bEnabled; }
    bool IsInputEnabled() const { return bInputEnabled; }

public:
    FOnMoveForward OnMoveForward;
    FOnMoveRight OnMoveRight;
    FOnTurn OnTurn;
    FOnLookUp OnLookUp;

private:
    void HandleKeyboardInput(float DeltaTime);
    void HandleMouseInput(float DeltaTime);

    bool bInputEnabled = true;  // Shift + F1 토글용
};
