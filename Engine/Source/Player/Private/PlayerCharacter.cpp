#include "pch.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
#include "Component/Collision/Public/SphereComponent.h"
#include "Component/Collision/Public/ShapeComponent.h"
#include "Component/Public/SceneComponent.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
IMPLEMENT_CLASS(APlayerCharacter, APawn)

APlayerCharacter::APlayerCharacter()
{
	UE_LOG("===========================================");
	UE_LOG("[PlayerCharacter] CONSTRUCTOR CALLED!");
	UE_LOG("===========================================");

	bCanEverTick = true;
	MovementSpeed = 100.0f;
	this->AddTag("Player");
	// StaticMesh 추가
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (!StaticMeshComponent)
	{
		UE_LOG_ERROR("ACharacter: Failed to create StaticMeshComponent");
	}
	else
	{
		SetRootComponent(StaticMeshComponent);
		// Mesh 설정 (구체로 표시)
		StaticMeshComponent->SetStaticMesh("Data/MIG_29.obj");
		StaticMeshComponent->SetRelativeScale3D(FVector(10.0f, 10.0f, 10.0f));  // 크기 조정
	}

	// 구체 충돌 컴포넌트
	CollisionComponent = CreateDefaultSubobject<USphereComponent>();
	if (!CollisionComponent)
	{
		UE_LOG_ERROR("APlayerCharacter: Failed to create CollisionComponent");
	}
	else
	{
		CollisionComponent->AttachToComponent(StaticMeshComponent);
		CollisionComponent->bGenerateHitEvents = true;
		CollisionComponent->bGenerateOverlapEvents = true;
		CollisionComponent->bBlockComponent = true;
		CollisionComponent->SetRelativeLocation(FVector(-2.0f, 0.0f, 2.2f));
		CollisionComponent->SetSphereRadius(3.0f);  // 전투기 전체를 커버하는 구체
		// 충돌 콜백 등록
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnEndOverlap);
		CollisionComponent->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnHit);
		UE_LOG("[PlayerCharacter] SphereCollision created");
	}

	// CollisionComp를 APawn에서 생성 후 RootComp로 지정
	// Create RootComponent first (required for Actor Transform)
	// USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>();
	// SetRootComponent(RootComp);

	// ✅ Lua 스크립트 활성화 (BeginPlay에서 PlayerWeapon 로드)
	SetUseScript(true);

	UE_LOG("[PlayerCharacter] Constructor: RootComponent=%p, MeshComponent=%p", CollisionComponent, StaticMeshComponent);
}

APlayerCharacter::~APlayerCharacter()
{
}

void APlayerCharacter::BeginPlay()
{
	UE_LOG("===========================================");
	UE_LOG("[PlayerCharacter] BEGINPLAY CALLED!");
	UE_LOG("===========================================");

	// Lua 스크립트 초기화 (Super::BeginPlay 전에!)
	ULuaScriptComponent* LuaComp = GetLuaScriptComponent();
	if (!LuaComp)
	{
		UE_LOG("[PlayerCharacter] Initializing LuaScriptComponent...");
		InitLuaScriptComponent();
		LuaComp = GetLuaScriptComponent();
	}

	if (LuaComp)
	{
		UE_LOG("[PlayerCharacter] Setting script name to PlayerWeapon...");
		LuaComp->SetScriptName(FString("PlayerWeapon"));
		UE_LOG("[PlayerCharacter] PlayerWeapon script name set!");
	}
	else
	{
		UE_LOG_ERROR("[PlayerCharacter] Failed to get LuaScriptComponent!");
	}

	// 이제 Super::BeginPlay 호출 (이미 스크립트가 설정되어 있음)
	Super::BeginPlay();

	// 충돌 컴포넌트를 Level에 등록
	if (CollisionComponent)
	{
		RegisterComponent(CollisionComponent);
		UE_LOG("[PlayerCharacter] CollisionComponent registered to Level");
	}

	UE_LOG("[PlayerCharacter] BeginPlay: %s at (%.1f, %.1f, %.1f)",
		GetName().ToString().c_str(),
		GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);

	UE_LOG("===========================================");
	UE_LOG("[PlayerCharacter] BeginPlay COMPLETE!");
	UE_LOG("===========================================");
	//RegisterComponent(CollisionComponent);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 카메라 쉐이크 업데이트
	if (bIsCameraShaking)
	{
		CameraShakeTimer += DeltaTime;

		if (CameraShakeTimer >= CameraShakeDuration)
		{
			// 쉐이크 종료 - 원래 위치로 복원
			bIsCameraShaking = false;
			CameraShakeTimer = 0.0f;
			SetActorLocation(OriginalCameraOffset);
		}
		else
		{
			// 랜덤 오프셋 적용 (흔들림 효과)
			float shakeAmount = CameraShakeIntensity * (1.0f - (CameraShakeTimer / CameraShakeDuration)); // 점점 약해짐
			float randomX = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * shakeAmount;
			float randomY = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * shakeAmount;
			float randomZ = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * shakeAmount;

			SetActorLocation(OriginalCameraOffset + FVector(randomX, randomY, randomZ));
		}
	}
}

void APlayerCharacter::MoveForward(float Value)
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

	FVector NewLocation = GetActorLocation() + (Forward * Value * MovementSpeed * 10);
	SetActorLocation(NewLocation);
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// A/D: Roll 회전 (A = 왼쪽 기울기, D = 오른쪽 기울기)
	FQuaternion CurrentRotation = GetActorRotation();
	FVector CurrentEuler = CurrentRotation.ToEuler();

	// Roll 회전 적용 (Value already contains DeltaTime)
	float RollDelta = Value * RotationSpeed;
	CurrentEuler.X += RollDelta;  // X = Roll

	FQuaternion NewRotation = FQuaternion::FromEuler(CurrentEuler);
	SetActorRotation(NewRotation);
}

void APlayerCharacter::Turn(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// 마우스 좌우: Yaw 회전 (왼쪽 = 마이너스, 오른쪽 = 플러스)
	FQuaternion CurrentRotation = GetActorRotation();
	FVector CurrentEuler = CurrentRotation.ToEuler();

	// Yaw 회전 적용 (Value already contains DeltaTime)
	float YawDelta = Value * MouseSensitivity;
	CurrentEuler.Z += YawDelta;  // Z = Yaw

	FQuaternion NewRotation = FQuaternion::FromEuler(CurrentEuler);
	SetActorRotation(NewRotation);
}

void APlayerCharacter::LookUp(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	// 마우스 상하: Pitch 회전 (위 = 플러스, 아래 = 마이너스)
	FQuaternion CurrentRotation = GetActorRotation();
	FVector CurrentEuler = CurrentRotation.ToEuler();

	// Pitch 회전 적용 (Value already contains DeltaTime)
	float PitchDelta = Value * MouseSensitivity;
	CurrentEuler.Y += PitchDelta;  // Y = Pitch

	FQuaternion NewRotation = FQuaternion::FromEuler(CurrentEuler);
	SetActorRotation(NewRotation);
}

void APlayerCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ULuaScriptComponent* LuaComp = this->GetLuaScriptComponent())
	{
		LuaComp->ActivateFunction("OnBeginOverlap", OverlappedComp, OtherActor,
			OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
		//LuaComp->ActivateFunction("OnBeginOverlap");
	}
	UE_LOG("Player Character Begin Overlap");
}

void APlayerCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex)
{
	if (ULuaScriptComponent* LuaComp = this->GetLuaScriptComponent())
	{
		LuaComp->ActivateFunction("OnEndOverlap", OverlappedComp, OtherActor,
			OtherComp, OtherBodyIndex);
	}
	UE_LOG("Player Character End Overlap");
}

void APlayerCharacter::OnHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& OutHit)
{
	if (ULuaScriptComponent* LuaComp = this->GetLuaScriptComponent())
	{
		UE_LOG("Character Hit Lua active");
		LuaComp->ActivateFunction("OnHit", OverlappedComp, OtherActor,
			OtherComp, NormalImpulse, OutHit);
	}
	UE_LOG("Player Character Hit");
}

void APlayerCharacter::StartCameraShake(float Intensity, float Duration)
{
	// 현재 위치 저장
	OriginalCameraOffset = GetActorLocation();

	// 쉐이크 파라미터 설정
	CameraShakeIntensity = Intensity;
	CameraShakeDuration = Duration;
	CameraShakeTimer = 0.0f;
	bIsCameraShaking = true;

	UE_LOG("[PlayerCharacter] Camera shake started: Intensity=%.1f, Duration=%.1f", Intensity, Duration);
}

