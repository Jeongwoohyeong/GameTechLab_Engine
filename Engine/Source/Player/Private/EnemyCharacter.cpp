#include "pch.h"
#include "Player/Public/EnemyCharacter.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/BoxComponent.h"
#include "Component/Collision/Public/CapsuleComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "Manager/Time/Public/TimeManager.h"
#include "Level/Public/World.h"

IMPLEMENT_CLASS(AEnemyCharacter, APawn)

AEnemyCharacter::AEnemyCharacter()
{
	bCanEverTick = true;
	UE_LOG("[EnemyCharacter] Constructor called");
}

UClass* AEnemyCharacter::GetDefaultRootComponent()
{
	return USceneComponent::StaticClass();
}

void AEnemyCharacter::InitializeComponents()
{
	Super::InitializeComponents();

	UE_LOG("[EnemyCharacter] InitializeComponents called - RootComponent=%p", GetRootComponent());

	// StaticMesh 컴포넌트 생성
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (StaticMeshComponent)
	{
		StaticMeshComponent->AttachToComponent(GetRootComponent());
		StaticMeshComponent->SetStaticMesh("Data/Enemy_F-14.obj");
		StaticMeshComponent->SetRelativeScale3D(FVector(30.0f, 30.0f, 30.0f));
		UE_LOG("[EnemyCharacter] StaticMeshComponent created and attached");
	}

	// ========================================
	// 충돌 컴포넌트들 생성
	// ========================================

	// 1. 비행기 몸체 충돌 (캡슐)
	BodyCollision = CreateDefaultSubobject<UCapsuleComponent>();
	if (BodyCollision)
	{
		BodyCollision->AttachToComponent(GetRootComponent());
		BodyCollision->SetCapsuleRadius(50.0f);  // 임시 값 (나중에 조절)
		BodyCollision->SetCapsuleHalfHeight(100.0f);  // 임시 값 (나중에 조절)
		BodyCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));  // 임시 위치
		BodyCollision->bGenerateOverlapEvents = true;
		BodyCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
		BodyCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
		BodyCollision->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
		UE_LOG("[EnemyCharacter] BodyCollision (Capsule) created and attached");
	}

	// 2. 왼쪽 날개 충돌 (박스)
	LeftWingCollision = CreateDefaultSubobject<UBoxComponent>();
	if (LeftWingCollision)
	{
		LeftWingCollision->AttachToComponent(GetRootComponent());
		LeftWingCollision->SetBoxExtent(FVector(50.0f, 100.0f, 10.0f));  // 임시 크기
		LeftWingCollision->SetRelativeLocation(FVector(0.0f, -100.0f, 0.0f));  // 임시 위치 (왼쪽)
		LeftWingCollision->bGenerateOverlapEvents = true;
		LeftWingCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
		LeftWingCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
		LeftWingCollision->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
		UE_LOG("[EnemyCharacter] LeftWingCollision (Box) created and attached");
	}

	// 3. 오른쪽 날개 충돌 (박스)
	RightWingCollision = CreateDefaultSubobject<UBoxComponent>();
	if (RightWingCollision)
	{
		RightWingCollision->AttachToComponent(GetRootComponent());
		RightWingCollision->SetBoxExtent(FVector(50.0f, 100.0f, 10.0f));  // 임시 크기
		RightWingCollision->SetRelativeLocation(FVector(0.0f, 100.0f, 0.0f));  // 임시 위치 (오른쪽)
		RightWingCollision->bGenerateOverlapEvents = true;
		RightWingCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
		RightWingCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
		RightWingCollision->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
		UE_LOG("[EnemyCharacter] RightWingCollision (Box) created and attached");
	}

	UE_LOG("[EnemyCharacter] InitializeComponents complete - All collision components created");
}

AEnemyCharacter::~AEnemyCharacter()
{
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG("[EnemyCharacter] BeginPlay - Enemy spawned");
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// AI나 Lua 스크립트에서 제어할 수 있도록 기본 Tick만 제공
}

void AEnemyCharacter::MoveForward(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// Actor의 Forward 방향 계산 (로컬 좌표계)
	FQuaternion Rotation = GetActorRotation();
	FMatrix RotMatrix = Rotation.ToRotationMatrix();
	FVector Forward(RotMatrix.Data[0][0], RotMatrix.Data[0][1], RotMatrix.Data[0][2]);
	Forward.Normalize();

	FVector NewLocation = GetActorLocation() + (Forward * Value * MovementSpeed);
	SetActorLocation(NewLocation);
}

void AEnemyCharacter::MoveRight(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// Actor의 Right 방향 계산 (로컬 좌표계)
	FQuaternion Rotation = GetActorRotation();
	FMatrix RotMatrix = Rotation.ToRotationMatrix();
	FVector Right(RotMatrix.Data[1][0], RotMatrix.Data[1][1], RotMatrix.Data[1][2]);
	Right.Normalize();

	FVector NewLocation = GetActorLocation() + (Right * Value * MovementSpeed);
	SetActorLocation(NewLocation);
}

void AEnemyCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	UE_LOG("[EnemyCharacter] OnBeginOverlap with: %s", OtherActor->GetName().ToString().c_str());

	// Lua 스크립트가 있으면 이벤트 전달
	if (ULuaScriptComponent* LuaScript = GetLuaScriptComponent())
	{
		LuaScript->ActivateFunction("OnBeginOverlap", OtherActor);
	}
}

void AEnemyCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	UE_LOG("[EnemyCharacter] OnEndOverlap with: %s", OtherActor->GetName().ToString().c_str());

	// Lua 스크립트가 있으면 이벤트 전달
	if (ULuaScriptComponent* LuaScript = GetLuaScriptComponent())
	{
		LuaScript->ActivateFunction("OnEndOverlap", OtherActor);
	}
}

void AEnemyCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor)
	{
		return;
	}

	UE_LOG("[EnemyCharacter] OnHit with: %s", OtherActor->GetName().ToString().c_str());

	// Lua 스크립트가 있으면 이벤트 전달
	if (ULuaScriptComponent* LuaScript = GetLuaScriptComponent())
	{
		LuaScript->ActivateFunction("OnHit", OtherActor);
	}
}
