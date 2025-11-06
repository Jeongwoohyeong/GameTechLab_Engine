#pragma once
#include "Actor/Public/Actor.h"
#include "Physics/Public/AABB.h"

class AMissileActor;
class APlayerCameraManager;
class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;
struct FHitResult;

/**
 * @brief Base actor for enemies that can trigger cinematic slow-motion effects
 * when a missile is about to hit them
 */
UCLASS()
class AEnemyBaseActor : public AActor
{
    GENERATED_BODY()
    DECLARE_CLASS(AEnemyBaseActor, AActor)

public:
    AEnemyBaseActor();
    virtual ~AEnemyBaseActor() = default;

    // AActor interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /**
     * @brief Get the AABB bounding box for this enemy
     */
    FAABB GetBoundingBox() const { return BoundingBox; }

    /**
     * @brief Update the bounding box based on mesh components
     */
    void UpdateBoundingBox();

    /**
     * @brief Check if this enemy is currently targeted by a cinematic
     */
    bool IsTargeted() const { return bIsTargeted; }

    /**
     * @brief Reset the targeted state (call when cinematic ends)
     */
    void ResetTargetState() { bIsTargeted = false; }

    /**
     * @brief Get mesh component to set mesh and scale
     */
    UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

    /**
     * @brief Get collision component to adjust radius
     */
    USphereComponent* GetCollisionComponent() const { return CollisionComponent; }

    /**
     * @brief Called when hit by a missile
     */
    void OnHitByMissile(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
    /**
     * @brief Trigger cinematic slow-motion and effects
     */
    void TriggerCinematic();

    /**
     * @brief End the cinematic effect
     */
    void EndCinematic();

private:
    /** Static mesh component */
    UStaticMeshComponent* MeshComponent = nullptr;

    /** Collision component */
    USphereComponent* CollisionComponent = nullptr;

    /** AABB bounding box for ray-intersection tests */
    FAABB BoundingBox;

    /** Is this enemy currently being targeted by a cinematic? */
    bool bIsTargeted = false;

    /** How far ahead to check for incoming missiles (in world units) */
    float MissileCheckDistance = 2000.0f;

    /** Minimum time before impact to trigger cinematic (in seconds) */
    float MinTimeBeforeImpact = 0.5f;

    /** Duration of slow motion effect (in real-time seconds) */
    float SlowMotionDuration = 3.0f;

    /** Slow motion speed multiplier (0.0 - 1.0) */
    float SlowMotionSpeed = 0.2f;

    /** Camera transition duration (in seconds) */
    float CameraTransitionDuration = 0.5f;

    /** Cinematic timer */
    float CinematicTimer = 0.0f;

    /** Reference to camera manager */
    APlayerCameraManager* CameraManager = nullptr;
};
