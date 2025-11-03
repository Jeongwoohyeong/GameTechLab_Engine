#pragma once
#include "Actor/Public/Actor.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class UPrimitiveComponent;
struct FHitResult;

/**
 * @brief 미사일 액터 클래스
 * 캡슐 충돌 컴포넌트를 가진 미사일
 */
UCLASS()
class AMissileActor : public AActor
{
	GENERATED_BODY()
	DECLARE_CLASS(AMissileActor, AActor)

public:
	AMissileActor();
	virtual ~AMissileActor();

	// 충돌 컴포넌트 접근자 (크기/위치 조절용)
	UCapsuleComponent* GetMissileCollision() const { return MissileCollision; }

	// 충돌 콜백
	void OnMissileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

private:
	// 컴포넌트들
	UStaticMeshComponent* StaicMeshComponent = nullptr;
	UCapsuleComponent* MissileCollision = nullptr;  // 미사일 충돌
};
