#include "pch.h"
#include "GamePlay/Public/PlayerController.h"
#include "GamePlay/public/PlayerInput.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"

IMPLEMENT_CLASS(APlayerController, AActor)

APlayerController::APlayerController()
{
    bCanEverTick = true;
}

APlayerController::~APlayerController()
{
}

void APlayerController::Initialize()
{
    PlayerInput = NewObject<UPlayerInput>();

    SetInput(PlayerInput);
}

void APlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PlayerInput)
    {
        PlayerInput->Update(DeltaTime);
    }
}

void APlayerController::SetInput(UPlayerInput* PlayerInput)
{
    PlayerInput->OnMoveForward.AddDynamic(this, &APlayerController::MoveForward);
    PlayerInput->OnMoveRight.AddDynamic(this, &APlayerController::MoveRight);
    PlayerInput->OnTurn.AddDynamic(this, &APlayerController::Turn);
    PlayerInput->OnLookUp.AddDynamic(this, &APlayerController::LookUp);
}

void APlayerController::Possess(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG("Target Actor is null");
        return;
    }
    ControlledActor.Set(TargetActor);

    UE_LOG("[PlayerController] Possessed: %s", TargetActor->GetName().ToString().c_str());
}

void APlayerController::UnPossess()
{
    ControlledActor.Reset();
}

void APlayerController::MoveForward(float Value)
{
    if (!ControlledActor.IsValid())
    {
        return;
    }

    // Cast to APawn and call its virtual function
    if (APawn* PawnActor = Cast<APawn>(ControlledActor.Get()))
    {
        PawnActor->MoveForward(Value);
    }
}

void APlayerController::MoveRight(float Value)
{
    if (!ControlledActor.IsValid())
    {
        return;
    }

    // Cast to APawn and call its virtual function
    if (APawn* PawnActor = Cast<APawn>(ControlledActor.Get()))
    {
        PawnActor->MoveRight(Value);
    }
}

void APlayerController::Turn(float Value)
{
    if (!ControlledActor.IsValid())
    {
        return;
    }

    // Cast to APawn and call its virtual function
    if (APawn* PawnActor = Cast<APawn>(ControlledActor.Get()))
    {
        PawnActor->Turn(Value);
    }
}

void APlayerController::LookUp(float Value)
{
    if (!ControlledActor.IsValid())
    {
        return;
    }

    // Cast to APawn and call its virtual function
    if (APawn* PawnActor = Cast<APawn>(ControlledActor.Get()))
    {
        PawnActor->LookUp(Value);
    }
}
