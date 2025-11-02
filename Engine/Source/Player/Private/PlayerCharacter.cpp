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

	UE_LOG("[PlayerCharacter] BeginPlay: %s at (%.1f, %.1f, %.1f)",
		GetName().ToString().c_str(),
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
}

