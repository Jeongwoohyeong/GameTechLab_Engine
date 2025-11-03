#include "pch.h"
#include "Player/Public/EnemyCharacter.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/SphereComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "Manager/Time/Public/TimeManager.h"
#include "Level/Public/World.h"

IMPLEMENT_CLASS(AEnemyCharacter, APawn)

AEnemyCharacter::AEnemyCharacter()
{
	bCanEverTick = true;
	UE_LOG("[EnemyCharacter] Constructor called");
	this->AddTag("Enemy");
	// StaticMesh 컴포넌트 생성
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (!StaticMeshComponent)
	{
		UE_LOG_ERROR("AEnemyCharacter: Failed to create StaticMeshComponent");
	}
	else
	{
		SetRootComponent(StaticMeshComponent);
		StaticMeshComponent->SetStaticMesh("Data/Enemy_F-14.obj");
		StaticMeshComponent->SetRelativeScale3D(FVector(30.0f, 30.0f, 30.0f));
		// ✅ StaticMeshComponent는 충돌 처리하지 않으므로 Tick 비활성화
		StaticMeshComponent->SetCanEverTick(false);
		StaticMeshComponent->bGenerateOverlapEvents = false;
		UE_LOG("[EnemyCharacter] StaticMeshComponent created and attached");
	}

	// 구체 충돌 컴포넌트
	CollisionComponent = CreateDefaultSubobject<USphereComponent>();
	if (!CollisionComponent)
	{
		UE_LOG_ERROR("AEnemyCharacter: Failed to create CollisionComponent");
	}
	else
	{
		CollisionComponent->SetRelativeLocation(FVector(-2.f, 0.0f, 0.0f));
		CollisionComponent->AttachToComponent(StaticMeshComponent);
		CollisionComponent->SetSphereRadius(1.0f);  // 적 기체를 커버하는 구체
		CollisionComponent->bGenerateHitEvents = true;
		CollisionComponent->bGenerateOverlapEvents = true;
		CollisionComponent->bBlockComponent = true;
		// 충돌 콜백 등록
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
		CollisionComponent->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
		UE_LOG("[EnemyCharacter] SphereCollision created");
	}
	
	// ✅ Lua 스크립트 활성화 (데미지 처리용)
	SetUseScript(true);
	InitLuaScriptComponent();
	if (ULuaScriptComponent* LuaComponent = GetLuaScriptComponent())
	{
		LuaComponent->SetScriptName("Scripts/Enemy/Enemy.lua");
		UE_LOG("[EnemyCharacter] Script name set to Enemy.lua");
	}
}

AEnemyCharacter::~AEnemyCharacter()
{
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 컴포넌트를 Level에 등록
	if (CollisionComponent)
	{
		RegisterComponent(CollisionComponent);
		UE_LOG("[EnemyCharacter] CollisionComponent registered to Level");
	}

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

	// 적 캐릭터끼리 충돌 무시
	FString OtherName = OtherActor->GetName().ToString();
	if (OtherName.find("AEnemyCharacter") != std::string::npos)
	{
		return;
	}

	// Lua 스크립트가 있으면 이벤트 전달
	if (ULuaScriptComponent* LuaScript = GetLuaScriptComponent())
	{
		LuaScript->ActivateFunction("OnBeginOverlap", OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}

void AEnemyCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	// 적 캐릭터끼리 충돌 무시
	FString OtherName = OtherActor->GetName().ToString();
	if (OtherName.find("AEnemyCharacter") != std::string::npos)
	{
		return;
	}

	// Lua 스크립트가 있으면 이벤트 전달
	if (ULuaScriptComponent* LuaScript = GetLuaScriptComponent())
	{
		LuaScript->ActivateFunction("OnEndOverlap", OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	}
}

void AEnemyCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor)
	{
		return;
	}

	// 적 캐릭터끼리 충돌 무시
	FString OtherName = OtherActor->GetName().ToString();
	if (OtherName.find("AEnemyCharacter") != std::string::npos)
	{
		return;
	}

	// Lua 스크립트가 있으면 이벤트 전달
	if (ULuaScriptComponent* LuaScript = GetLuaScriptComponent())
	{
		LuaScript->ActivateFunction("OnHit", HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
	}
}
