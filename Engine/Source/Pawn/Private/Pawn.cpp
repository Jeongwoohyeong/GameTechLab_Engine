#include "pch.h"
#include "Pawn/Public/Pawn.h"

#include "Component/Collision/Public/SphereComponent.h"
#include "GamePlay/Public/PlayerController.h"

IMPLEMENT_CLASS(APawn, AActor)

APawn::APawn()
{
	bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(FName("CollisionComponent"));
	if (!CollisionComponent)
	{
		UE_LOG_ERROR("APawn: Failed to create CollisionComponent");
	}
	else
	{
		SetRootComponent(CollisionComponent);
		CollisionComponent->bGenerateHitEvents = true;
		CollisionComponent->bGenerateOverlapEvents = true;
		CollisionComponent->bBlockComponent = true;
		Cast<USphereComponent>(CollisionComponent)->SetSphereRadius(15.0f);
		UE_LOG("APawn Create Collision Component");
	}	
}

APawn::~APawn()
{
}

void APawn::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[Pawn] BeginPlay: %s", GetName().ToString().c_str());
}

void APawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APawn::PossessedBy(APlayerController* NewController)
{
	if (!NewController)
	{
		UE_LOG_WARNING("[Pawn] Attempted to possess with null controller");
		return;
	}

	// Unpossess current controller if any
	if (Controller.IsValid())
	{
		UnPossessed();
	}

	Controller.Set(NewController);
	UE_LOG("[Pawn] %s possessed by Controller", GetName().ToString().c_str());
}

void APawn::UnPossessed()
{
	if (Controller.IsValid())
	{
		UE_LOG("[Pawn] %s unpossessed", GetName().ToString().c_str());
		Controller.Reset();
	}
}

APlayerController* APawn::GetController() const
{
	return Controller.Get();
}

