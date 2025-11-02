#include "pch.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"

#include "Component/Collision/Public/ShapeComponent.h"
#include "Component/Public/SceneComponent.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Public/ULuaScriptComponent.h"

IMPLEMENT_CLASS(APlayerCharacter, APawn)

APlayerCharacter::APlayerCharacter()
{
	bCanEverTick = true;
	MovementSpeed = 100.0f;

	// CollisionComp를 APawn에서 생성 후 RootComp로 지정
	// Create RootComponent first (required for Actor Transform)
	// USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>();
	// SetRootComponent(RootComp);

	// StaticMesh 추가
	UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>();
	MeshComp->AttachToComponent(CollisionComponent);

	// Mesh 설정 (구체로 표시)
	MeshComp->SetStaticMesh("Data/Shapes/Sphere.obj");
	MeshComp->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));  // 크기 조정

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnEndOverlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnHit);

	UE_LOG("[PlayerCharacter] Constructor: RootComponent=%p, MeshComponent=%p", CollisionComponent, MeshComp);
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

void APlayerCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ULuaScriptComponent* ScriptComponent = GetLuaScriptComponent())
	{
		ScriptComponent->ActivateFunction("OnBeginOverlap", OverlappedComp, OtherActor,
		                                  OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
	UE_LOG("Player Character Begin Overlap");
}

void APlayerCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex)
{
	if (ULuaScriptComponent* ScriptComponent = GetLuaScriptComponent())
	{
		ScriptComponent->ActivateFunction("OnEndOverlap", OverlappedComp, OtherActor,
		                                  OtherComp, OtherBodyIndex);
	}
	UE_LOG("Player Character End Overlap");
}

void APlayerCharacter::OnHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& OutHit)
{
	if (ULuaScriptComponent* ScriptComponent = GetLuaScriptComponent())
	{
		ScriptComponent->ActivateFunction("OnHit", OverlappedComp, OtherActor,
		                                  OtherComp, NormalImpulse, OutHit);
	}
	UE_LOG("Player Character Hit");
}

