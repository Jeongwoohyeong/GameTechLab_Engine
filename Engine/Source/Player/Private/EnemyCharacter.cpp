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

	// 몸통 캡슐 충돌 컴포넌트
	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>();
	if (!CollisionComponent)
	{
		UE_LOG_ERROR("AEnemyCharacter: Failed to create CollisionComponent");
	}
	else
	{
		CollisionComponent->SetRelativeLocation(FVector(-1.35f, 0.0f, 0.0f));
		CollisionComponent->SetRelativeRotation(FQuaternion::FromEuler(FVector(0.0f, 90.0f, 0.0f)));
		CollisionComponent->AttachToComponent(StaticMeshComponent);
		CollisionComponent->SetCapsuleHalfHeight(1.4f);
		CollisionComponent->SetCapsuleRadius(0.4f);
		CollisionComponent->bGenerateHitEvents = true;
		CollisionComponent->bGenerateOverlapEvents = true;
		CollisionComponent->bBlockComponent = true;
		// 충돌 콜백 등록
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
		CollisionComponent->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
		UE_LOG("[EnemyCharacter] Body CapsuleCollision created");
	}

	// 날개 박스 충돌 컴포넌트
	WingCollision = CreateDefaultSubobject<UBoxComponent>();
	if (!WingCollision)
	{
		UE_LOG_ERROR("AEnemyCharacter: Failed to create WingCollision");
	}
	else
	{
		WingCollision->AttachToComponent(StaticMeshComponent);
		// 날개 크기에 맞춰 박스 설정 (X=전후 두께, Y=좌우 날개 길이, Z=상하 두께)
		WingCollision->SetBoxExtent(FVector(0.8f, 2.0f, 0.2f));  // 임시 크기, 나중에 조정 가능
		WingCollision->SetRelativeLocation(FVector(-2.8f, 0.0f, 0.0f));
		WingCollision->bGenerateHitEvents = true;
		WingCollision->bGenerateOverlapEvents = true;
		WingCollision->bBlockComponent = true;
		// 충돌 콜백 등록 (동일한 콜백 사용)
		WingCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnBeginOverlap);
		WingCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnEndOverlap);
		WingCollision->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);
		UE_LOG("[EnemyCharacter] Wing BoxCollision created");
	}
	
	// ✅ Lua 스크립트 활성화 (데미지 처리용)
	SetUseScript(true);
	if (ULuaScriptComponent* LuaComponent = GetLuaScriptComponent())
	{
		LuaComponent->SetScriptName("Scripts/Enemy/Enemy.lua");

		if (!LuaComponent->LoadScript())
		{
			UE_LOG_ERROR("[EnemyCharacter] Failed to load Enemy.lua");
		}
		else
		{
			UE_LOG("[EnemyCharacter] Successfully loaded Enemy.lua");
		}
	}
}

AEnemyCharacter::~AEnemyCharacter()
{
}

void AEnemyCharacter::BeginPlay()
{
	// ✅ Lua 스크립트 초기화 (Super::BeginPlay 전에)
	if (IsUsingScript())
	{
		ULuaScriptComponent* LuaComp = GetLuaScriptComponent();
		if (!LuaComp)
		{
			InitLuaScriptComponent();
			LuaComp = GetLuaScriptComponent();
		}

		if (LuaComp)
		{
			LuaComp->SetScriptName(FString("EnemyCharacter"));
			UE_LOG("[EnemyCharacter] Lua script 'EnemyCharacter' loaded");
		}
		else
		{
			UE_LOG_ERROR("[EnemyCharacter] Failed to initialize Lua script component");
		}
	}

	Super::BeginPlay();

	// 충돌 컴포넌트들을 Level에 등록
	if (CollisionComponent)
	{
		RegisterComponent(CollisionComponent);
		UE_LOG("[EnemyCharacter] CollisionComponent registered to Level");
	}
	if (WingCollision)
	{
		RegisterComponent(WingCollision);
		UE_LOG("[EnemyCharacter] WingCollision registered to Level");
	}

	UE_LOG("[EnemyCharacter] BeginPlay - Enemy spawned");
	//RegisterComponent(CollisionComponent);
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
		LuaScript->ActivateFunction("OnBeginOverlap", OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
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
		LuaScript->ActivateFunction("OnEndOverlap", OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
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
		LuaScript->ActivateFunction("OnHit", HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
	}
}
