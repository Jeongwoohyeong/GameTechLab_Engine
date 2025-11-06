#pragma once
#include "Actor/Public/Actor.h"

class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;
struct FHitResult;

/**
 * @brief 미사일 액터 클래스
 * 구체 충돌 컴포넌트를 가진 미사일
 */
UCLASS()
class AMissileActor : public AActor
{
	GENERATED_BODY()
	DECLARE_CLASS(AMissileActor, AActor)

public:
	AMissileActor();
	virtual ~AMissileActor();

	// Actor lifecycle
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 충돌 컴포넌트 접근자 (크기/위치 조절용)
	USphereComponent* GetMissileCollision() const { return MissileCollision; }

	/**
	 * @brief Get the current velocity of the missile
	 * @return Velocity vector (direction * speed)
	 */
	FVector GetVelocity() const { return MissileDirection * MissileSpeed; }

	// 충돌 콜백
	void OnMissileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Lua에서 설정 가능한 미사일 속도와 방향
	FVector MissileDirection = FVector(1.0f, 0.0f, 0.0f);
	float MissileSpeed = 3600.0f;

private:
	// C++ 전용 플래그 (Lua 접근 불가)
	bool bHasHitEnemy = false;

	// 컴포넌트들
	UStaticMeshComponent* StaicMeshComponent = nullptr;
	USphereComponent* MissileCollision = nullptr;  // 미사일 충돌
};
