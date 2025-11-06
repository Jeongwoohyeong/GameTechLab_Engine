#include "pch.h"
#include "Actor/Public/MissileActor.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/SphereComponent.h"
#include "Component/Public/SceneComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "Player/Public/PlayerCharacter.h"
#include "Player/Public/EnemyCharacter.h"
#include "Actor/Public/EnemyBaseActor.h"
#include "Actor/Public/ExplosionActor.h"
#include "GameMode/Public/GameMode.h"
#include "GamePlay/Public/PlayerController.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"
#include "Level/Public/Level.h"
#include "Level/Public/World.h"
#include "Manager/Time/Public/TimeManager.h"
#include "Core/Public/AudioEngine.h"

IMPLEMENT_CLASS(AMissileActor, AActor)

AMissileActor::AMissileActor()
{
	bCanEverTick = true;
	// 메쉬 컴포넌트 생성
	StaicMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (StaicMeshComponent)
	{
		SetRootComponent(StaicMeshComponent);
		StaicMeshComponent->SetStaticMesh("Data/Missile.obj");
		StaicMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		StaicMeshComponent->SetRelativeRotation(FQuaternion::FromEuler(FVector(0.0f, 0.0f, 0.0f)));
		StaicMeshComponent->SetRelativeScale3D(FVector(50.f, 200.f, 200.f));
		StaicMeshComponent->SetCanEverTick(false);
		StaicMeshComponent->bGenerateOverlapEvents = false;
	}
	// 구체 충돌 컴포넌트 생성
	MissileCollision = CreateDefaultSubobject<USphereComponent>();
	if (MissileCollision)
	{
		MissileCollision->AttachToComponent(StaicMeshComponent);
		MissileCollision->SetSphereRadius(0.03f);
		MissileCollision->SetRelativeLocation(FVector(0.45f, 0.0f, 0.0f));
		MissileCollision->bGenerateHitEvents = true;
		MissileCollision->bBlockComponent = true;  // ✅ Hit 이벤트를 위해 Blocking 활성화
		MissileCollision->SetCanEverTick(true);
		MissileCollision->OnComponentHit.AddDynamic(this, &AMissileActor::OnMissileHit);
	}
}

AMissileActor::~AMissileActor()
{
}

void AMissileActor::BeginPlay()
{
	Super::BeginPlay();

	// 플래그 초기화
	bHasHitEnemy = false;

	if (MissileCollision)
	{
		RegisterComponent(MissileCollision);
	}
}

void AMissileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Velocity is calculated from MissileDirection and MissileSpeed
	// which are set by Lua script
}

void AMissileActor::OnMissileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Null 체크
	if (!OtherActor)
	{
		return;
	}

	// 자기 자신과의 충돌 무시
	if (OtherActor == this)
	{
		return;
	}

	// 다른 미사일과의 충돌 무시
	if (Cast<AMissileActor>(OtherActor))
	{
		return;
	}

	// 플레이어와의 충돌 무시
	if (Cast<APlayerCharacter>(OtherActor))
	{
		return;
	}

	// 적 캐릭터인 경우 데미지 처리
	AEnemyCharacter* EnemyChar = Cast<AEnemyCharacter>(OtherActor);
	if (EnemyChar != nullptr && !bHasHitEnemy)
	{
		// 중복 히트 방지
		bHasHitEnemy = true;

		// 폭발 위치 저장 (적 캐릭터 위치)
		FVector ExplosionLocation = EnemyChar->GetActorLocation();

		// C++에서 직접 TakeDamage 호출
		float MissileDamage = 100.0f;
		EnemyChar->TakeDamage(MissileDamage);

		// 폭발음 재생
		FAudioEngine::GetInstance().PlaySFX("Data/Audio/Explosion.wav", 1.0f);

		// 폭발 이펙트 생성
		if (GWorld && GWorld->GetLevel())
		{
			AExplosionActor* Explosion = NewObject<AExplosionActor>();
			if (Explosion)
			{
				Explosion->SetActorLocation(ExplosionLocation);
				Explosion->SetExplosionScale(300.0f);
				GWorld->GetLevel()->AddActorToLevel(Explosion);
				Explosion->BeginPlay();
			}
		}

		// 미사일을 화면 밖으로 이동
		SetActorLocation(FVector(0, 0, -10000));
		return;
	}

	// EnemyBaseActor인 경우 - 폭발 + 슬로모션 + 카메라 transition
	AEnemyBaseActor* EnemyBase = Cast<AEnemyBaseActor>(OtherActor);
	if (EnemyBase != nullptr && !bHasHitEnemy)
	{
		// 중복 히트 방지
		bHasHitEnemy = true;

		// 폭발 위치 저장
		FVector ExplosionLocation = EnemyBase->GetActorLocation();
		FVector MissilePos = GetActorLocation();

		UE_LOG("[MissileActor] HIT ENEMYBASEACTOR! Triggering explosion + slow motion + camera!");

		// 1. 슬로모션 발동
		UTimeManager::GetInstance().StartSlowMotion(0.2f, 3.0f);

		// 2. 카메라 transition
		if (GWorld)
		{
			AGameMode* GameMode = GWorld->GetGameMode();
			if (GameMode)
			{
				APlayerController* PC = GameMode->GetPlayerController();
				if (PC)
				{
					APlayerCameraManager* CameraManager = PC->GetPlayerCameraManager();
					if (CameraManager)
					{
						// 카메라를 폭발 지점 근처로 이동
						FVector MissileToEnemy = (ExplosionLocation - MissilePos).GetNormalized();
						FVector UpVector(0.0f, 0.0f, 1.0f);
						FVector SideVector = MissileToEnemy.Cross(UpVector).GetNormalized();

						// 카메라 위치: 폭발 지점 옆에서 관찰
						float CameraDistance = 50.0f;
						FVector CameraPos = ExplosionLocation + SideVector * CameraDistance + UpVector * 20.0f;

						// 카메라가 폭발을 바라보도록
						FVector LookDirection = (ExplosionLocation - CameraPos).GetNormalized();
						float Yaw = atan2f(LookDirection.Y, LookDirection.X) * (180.0f / 3.14159265f);
						float Pitch = atan2f(LookDirection.Z, sqrtf(LookDirection.X * LookDirection.X + LookDirection.Y * LookDirection.Y)) * (180.0f / 3.14159265f);
						FRotator CameraRot(Pitch, Yaw, 0.0f);

						CameraManager->StartTransitionToLocation(
							CameraPos,
							CameraRot,
							0.5f,  // transition duration
							ECameraEaseType::EaseInOut
						);

						UE_LOG("[MissileActor] Camera transition to (%.1f, %.1f, %.1f)",
							CameraPos.X, CameraPos.Y, CameraPos.Z);
					}
				}
			}
		}

		// 3. 폭발음 재생
		FAudioEngine::GetInstance().PlaySFX("Data/Audio/Explosion.wav", 1.0f);

		// 4. 폭발 이펙트 생성
		if (GWorld && GWorld->GetLevel())
		{
			AExplosionActor* Explosion = NewObject<AExplosionActor>();
			if (Explosion)
			{
				Explosion->SetActorLocation(ExplosionLocation);
				Explosion->SetExplosionScale(300.0f);
				GWorld->GetLevel()->AddActorToLevel(Explosion);
				Explosion->BeginPlay();
			}
		}

		// 미사일을 화면 밖으로 이동
		SetActorLocation(FVector(0, 0, -10000));
	}
}
