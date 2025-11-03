#include "pch.h"
#include "Player/Public/EnemyCharacter.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/CapsuleComponent.h"
#include "Component/Collision/Public/BoxComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "Manager/Time/Public/TimeManager.h"
#include "Level/Public/World.h"

IMPLEMENT_CLASS(AEnemyCharacter, APawn)

AEnemyCharacter::AEnemyCharacter()
{
	bCanEverTick = true;
	UE_LOG("[EnemyCharacter] Constructor called");

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>();
	if (!CollisionComponent)
	{
		UE_LOG_ERROR("ACharacter: Failed to create CollisionComponent");
	}
	else
	{
		SetRootComponent(CollisionComponent);
		CollisionComponent->bGenerateHitEvents = true;
		CollisionComponent->bGenerateOverlapEvents = true;
		CollisionComponent->bBlockComponent = true;
		UE_LOG("ACharacter Create Collision Component");
	}
	// StaticMesh 추가 (APawn의 CollisionComponent 사용)
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	StaticMeshComponent->AttachToComponent(CollisionComponent);

	// 적 전용 메시 설정
	StaticMeshComponent->SetStaticMesh("Data/Enemy_F-14.obj");
	StaticMeshComponent->SetRelativeScale3D(FVector(30.0f, 30.0f, 30.0f));

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
	// 충돌 콜백 등록 (CollisionComponent 사용)
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
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

void AEnemyCharacter::MoveForward(float Value)// 충돌 콜백 등록 (CollisionComponent 사용)
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
