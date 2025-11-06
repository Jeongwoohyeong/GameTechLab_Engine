#include "pch.h"
#include "Actor/Public/EnemyBaseActor.h"
#include "Actor/Public/MissileActor.h"
#include "Actor/Public/ExplosionActor.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"
#include "Manager/Time/Public/TimeManager.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/SphereComponent.h"
#include "Level/Public/World.h"
#include "Level/Public/Level.h"
#include "GameMode/Public/GameMode.h"
#include "GamePlay/Public/PlayerController.h"
#include "Core/Public/AudioEngine.h"

IMPLEMENT_CLASS(AEnemyBaseActor, AActor)

AEnemyBaseActor::AEnemyBaseActor()
{
    UE_LOG("===========================================");
    UE_LOG("[EnemyBaseActor] CONSTRUCTOR CALLED!");
    UE_LOG("===========================================");

    bCanEverTick = true;

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
    if (MeshComponent)
    {
        SetRootComponent(MeshComponent);
        // Mesh can be set after spawning via SetStaticMesh()
        // Scale controlled by SetActorScale3D()
        UE_LOG("[EnemyBaseActor] MeshComponent created successfully");
    }
    else
    {
        UE_LOG_ERROR("[EnemyBaseActor] Failed to create MeshComponent!");
    }

    // Create collision component
    CollisionComponent = CreateDefaultSubobject<USphereComponent>();
    if (CollisionComponent && MeshComponent)
    {
        CollisionComponent->AttachToComponent(MeshComponent);
        // Larger radius for easier missile collision
        CollisionComponent->SetSphereRadius(20.0f);
        CollisionComponent->bGenerateHitEvents = true;
        CollisionComponent->bBlockComponent = true;
        // Register collision callback
        CollisionComponent->OnComponentHit.AddDynamic(this, &AEnemyBaseActor::OnHitByMissile);
        UE_LOG("[EnemyBaseActor] CollisionComponent created successfully");
    }
    else
    {
        UE_LOG_ERROR("[EnemyBaseActor] Failed to create CollisionComponent!");
    }

    UE_LOG("[EnemyBaseActor] Constructor finished");
}

void AEnemyBaseActor::BeginPlay()
{
    UE_LOG("===========================================");
    UE_LOG("[EnemyBaseActor] BEGIN PLAY CALLED!");
    UE_LOG("===========================================");

    Super::BeginPlay();

    // Register collision component
    if (CollisionComponent)
    {
        RegisterComponent(CollisionComponent);
        UE_LOG("[EnemyBaseActor] CollisionComponent registered");
    }

    // Get camera manager reference
    if (GWorld)
    {
        AGameMode* GameMode = GWorld->GetGameMode();
        if (GameMode)
        {
            if (APlayerController* PC = GameMode->GetPlayerController())
            {
                CameraManager = PC->GetPlayerCameraManager();
                UE_LOG("[EnemyBaseActor] CameraManager found!");
            }
            else
            {
                UE_LOG("[EnemyBaseActor] PlayerController not found!");
            }
        }
        else
        {
            UE_LOG("[EnemyBaseActor] GameMode not found!");
        }
    }
    else
    {
        UE_LOG("[EnemyBaseActor] GWorld is null!");
    }

    // Initialize bounding box
    UpdateBoundingBox();

    FVector MyLocation = GetActorLocation();
    UE_LOG("[EnemyBaseActor] BeginPlay - Position: (%.2f, %.2f, %.2f)", MyLocation.X, MyLocation.Y, MyLocation.Z);
    UE_LOG("[EnemyBaseActor] BeginPlay - BoundingBox: Min(%.2f, %.2f, %.2f) Max(%.2f, %.2f, %.2f)",
        BoundingBox.Min.X, BoundingBox.Min.Y, BoundingBox.Min.Z,
        BoundingBox.Max.X, BoundingBox.Max.Y, BoundingBox.Max.Z);
}

void AEnemyBaseActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Try to get camera manager if we don't have it yet
    if (!CameraManager && GWorld)
    {
        AGameMode* GameMode = GWorld->GetGameMode();
        if (GameMode)
        {
            if (APlayerController* PC = GameMode->GetPlayerController())
            {
                CameraManager = PC->GetPlayerCameraManager();
                if (CameraManager)
                {
                    UE_LOG("[EnemyBaseActor] CameraManager found!");
                }
            }
        }
    }

    // Update cinematic timer if active
    if (bIsTargeted)
    {
        CinematicTimer += DeltaTime;
        if (CinematicTimer >= SlowMotionDuration)
        {
            EndCinematic();
        }
    }
}

void AEnemyBaseActor::UpdateBoundingBox()
{
    // Get all static mesh components and calculate combined AABB
    FVector ActorPos = GetActorLocation();
    FVector MinExtent(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector MaxExtent(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    bool bFoundMesh = false;

    for (UActorComponent* Comp : GetOwnedComponents())
    {
        if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Comp))
        {
            // Simple AABB estimation based on mesh bounds
            // TODO: Get actual mesh bounds if available
            FVector MeshScale = MeshComp->GetRelativeScale3D();
            FVector Extent = FVector(5.0f, 5.0f, 5.0f) * MeshScale;  // Default size

            MinExtent.X = std::min(MinExtent.X, ActorPos.X - Extent.X);
            MinExtent.Y = std::min(MinExtent.Y, ActorPos.Y - Extent.Y);
            MinExtent.Z = std::min(MinExtent.Z, ActorPos.Z - Extent.Z);

            MaxExtent.X = std::max(MaxExtent.X, ActorPos.X + Extent.X);
            MaxExtent.Y = std::max(MaxExtent.Y, ActorPos.Y + Extent.Y);
            MaxExtent.Z = std::max(MaxExtent.Z, ActorPos.Z + Extent.Z);

            bFoundMesh = true;
        }
    }

    if (bFoundMesh)
    {
        BoundingBox = FAABB(MinExtent, MaxExtent);
    }
    else
    {
        // Default AABB if no mesh found
        FVector DefaultExtent(10.0f, 10.0f, 10.0f);
        BoundingBox = FAABB(ActorPos - DefaultExtent, ActorPos + DefaultExtent);
    }
}

void AEnemyBaseActor::OnHitByMissile(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Check if hit by a missile
    AMissileActor* Missile = Cast<AMissileActor>(OtherActor);
    if (!Missile)
    {
        return;  // Not a missile
    }

    // Already triggered, ignore multiple hits
    if (bIsTargeted)
    {
        return;
    }

    UE_LOG("[EnemyBaseActor] HIT BY MISSILE!");

    // Trigger cinematic effect
    TriggerCinematic();
}

void AEnemyBaseActor::TriggerCinematic()
{
    bIsTargeted = true;
    CinematicTimer = 0.0f;

    UE_LOG("[EnemyBaseActor] TRIGGERING CINEMATIC!");

    // 1. Start Slow Motion
    UTimeManager& TimeManager = UTimeManager::GetInstance();
    TimeManager.StartSlowMotion(SlowMotionSpeed, SlowMotionDuration);
    UE_LOG("[EnemyBaseActor] Slow motion started: %.2fx speed for %.2f seconds",
        SlowMotionSpeed, SlowMotionDuration);

    // 2. SubUV 폭발 이펙트 생성
    FVector ExplosionLocation = GetActorLocation();

    if (GWorld && GWorld->GetLevel())
    {
        AExplosionActor* Explosion = NewObject<AExplosionActor>();
        if (Explosion)
        {
            Explosion->SetActorLocation(ExplosionLocation);
            Explosion->SetExplosionScale(300.0f); // 폭발 크기 설정
            GWorld->GetLevel()->AddActorToLevel(Explosion);
            Explosion->BeginPlay();

            UE_LOG("[EnemyBaseActor] Explosion effect spawned at (%.1f, %.1f, %.1f)",
                ExplosionLocation.X, ExplosionLocation.Y, ExplosionLocation.Z);
        }
    }

    // 3. 폭발음 재생
    FAudioEngine::GetInstance().PlaySFX("Data/Audio/Explosion.wav", 1.0f);

    UE_LOG("[EnemyBaseActor] Cinematic triggered - SlowMo + Explosion active!");
}

void AEnemyBaseActor::EndCinematic()
{
    if (!bIsTargeted)
    {
        return;
    }

    UE_LOG("[EnemyBaseActor] ENDING CINEMATIC");

    bIsTargeted = false;
    CinematicTimer = 0.0f;

    // Stop slow motion (it should auto-stop, but just in case)
    UTimeManager& TimeManager = UTimeManager::GetInstance();
    TimeManager.StopSlowMotion();

    UE_LOG("[EnemyBaseActor] Cinematic ended - Actor remains alive");
}
