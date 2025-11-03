#include "pch.h"
#include "Pawn/Public/Pawn.h"
#include "Player/Public/PlayerCharacter.h"
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

	// Create RootComponent first (required for Actor Transform)
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>();
	SetRootComponent(RootComp);

	// StaticMesh 추가
	UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>();
	MeshComp->AttachToComponent(RootComp);

	// Mesh 설정 (구체로 표시)
	MeshComp->SetStaticMesh("Data/SU-37.obj");
	MeshComp->SetRelativeScale3D(FVector(5.5f, 5.5f, 5.5f));  // 크기 조정

	// Lua 스크립트 활성화 (무기 시스템 - 미사일 발사)
	SetUseScript(true);

	UE_LOG("[PlayerCharacter] Constructor: RootComponent=%p, MeshComponent=%p", RootComp, MeshComp);
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

	UE_LOG("[PlayerCharacter] BeginPlay: %s at (%.1f, %.1f, %.1f)",
		GetName().ToString().c_str(),
		GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);

	UE_LOG("===========================================");
	UE_LOG("[PlayerCharacter] BeginPlay COMPLETE!");
	UE_LOG("===========================================");
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

	// Actor의 Forward 방향 계산 (로컬 좌표계)
	FQuaternion Rotation = GetActorRotation();
	FMatrix RotMatrix = Rotation.ToRotationMatrix();
	FVector Forward(RotMatrix.Data[0][0], RotMatrix.Data[0][1], RotMatrix.Data[0][2]);
	Forward.Normalize();

	FVector NewLocation = GetActorLocation() + (Forward * Value * MovementSpeed);
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

