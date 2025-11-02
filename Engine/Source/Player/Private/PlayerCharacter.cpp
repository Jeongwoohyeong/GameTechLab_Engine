#include "pch.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
#include "Component/Public/SceneComponent.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"

IMPLEMENT_CLASS(APlayerCharacter, APawn)

APlayerCharacter::APlayerCharacter()
{
	bCanEverTick = true;
	MovementSpeed = 100.0f;

	// Create RootComponent first (required for Actor Transform)
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>();
	SetRootComponent(RootComp);

	// StaticMesh 추가
	UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>();
	MeshComp->AttachToComponent(RootComp);

	// Mesh 설정 (구체로 표시)
	MeshComp->SetStaticMesh("Data/Shapes/Sphere.obj");
	MeshComp->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));  // 크기 조정

	UE_LOG("[PlayerCharacter] Constructor: RootComponent=%p, MeshComponent=%p", RootComp, MeshComp);
}

APlayerCharacter::~APlayerCharacter()
{
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[PlayerCharacter] BeginPlay: %s", GetName().ToString().c_str());

	// FIX: Re-attach all components to the correct RootComponent
	// This is needed because PIE world duplication creates new RootComponent but old attach references remain
	for (auto* Comp : GetOwnedComponents())
	{
		if (auto* SceneComp = Cast<USceneComponent>(Comp))
		{
			if (SceneComp != GetRootComponent())
			{
				USceneComponent* OldParent = SceneComp->GetAttachParent();

				// If attached to something but not to current RootComponent, re-attach
				if (OldParent && OldParent != GetRootComponent())
				{
					UE_LOG("[PlayerCharacter] Re-attaching %s from %p to RootComponent %p",
						SceneComp->GetClass()->GetName().ToString().c_str(),
						OldParent,
						GetRootComponent());

					SceneComp->DetachFromComponent();
					SceneComp->AttachToComponent(GetRootComponent());
				}
			}
		}
	}

	UE_LOG("[PlayerCharacter] BeginPlay complete. GetActorLocation() returns: (%.1f, %.1f, %.1f)",
		GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// Value already contains DeltaTime from PlayerInput
	FVector Forward(1.0f, 0.0f, 0.0f);
	FVector NewLocation = GetActorLocation() + (Forward * Value * MovementSpeed);
	SetActorLocation(NewLocation);

	// Find StaticMeshComponent and check its WorldLocation
	for (auto* Comp : GetOwnedComponents())
	{
		if (auto* MeshComp = Cast<UStaticMeshComponent>(Comp))
		{
			FVector MeshWorldLoc = MeshComp->GetWorldLocation();
			UE_LOG("[PlayerCharacter] StaticMesh WorldLocation: (%.1f, %.1f, %.1f)",
				MeshWorldLoc.X, MeshWorldLoc.Y, MeshWorldLoc.Z);
			break;
		}
	}

	UE_LOG("[PlayerCharacter] MoveForward: Value=%.3f, NewLocation=(%.1f, %.1f, %.1f)",
		Value, NewLocation.X, NewLocation.Y, NewLocation.Z);
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// Value already contains DeltaTime from PlayerInput
	FVector Right(0.0f, 1.0f, 0.0f);
	FVector NewLocation = GetActorLocation() + (Right * Value * MovementSpeed);
	SetActorLocation(NewLocation);

	UE_LOG("[PlayerCharacter] MoveRight: Value=%.3f, NewLocation=(%.1f, %.1f, %.1f)",
		Value, NewLocation.X, NewLocation.Y, NewLocation.Z);
}

