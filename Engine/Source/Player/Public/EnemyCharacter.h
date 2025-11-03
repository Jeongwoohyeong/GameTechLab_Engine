#pragma once
#include "Pawn/Public/Pawn.h"

class UStaticMeshComponent;
class UPrimitiveComponent;
class UCapsuleComponent;
class UBoxComponent;
struct FHitResult;

/**
 * @brief 적 캐릭터 클래스
 * APlayerCharacter와 유사하지만 적 전용 기능만 포함
 */
UCLASS()
class AEnemyCharacter : public APawn
{
	DECLARE_CLASS(AEnemyCharacter, APawn)

public:
	AEnemyCharacter();
	virtual ~AEnemyCharacter() override;

	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	// Movement functions (AI나 스크립트에서 사용)
	void MoveForward(float Value);
	void MoveRight(float Value);

	// Collision callbacks (PlayerCharacter와 동일한 시그니처)
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void OnHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& OutHit);

	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }
	UBoxComponent* GetWingCollision() const { return WingCollision; }

private:
	UCapsuleComponent* CollisionComponent = nullptr;  // 몸통 충돌
	UBoxComponent* WingCollision = nullptr;            // 날개 충돌
	UStaticMeshComponent* StaticMeshComponent = nullptr;
	

	float MovementSpeed = 200.0f; // 적 기본 이동 속도
};
