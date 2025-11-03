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

	// Component initialization
	virtual UClass* GetDefaultRootComponent() override;
	virtual void InitializeComponents() override;

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

	// 충돌 컴포넌트 접근자 (크기/위치 조절용)
	UCapsuleComponent* GetBodyCollision() const { return BodyCollision; }
	UBoxComponent* GetLeftWingCollision() const { return LeftWingCollision; }
	UBoxComponent* GetRightWingCollision() const { return RightWingCollision; }

private:
	// 컴포넌트들
	UStaticMeshComponent* StaticMeshComponent = nullptr;
	UCapsuleComponent* BodyCollision = nullptr;    // 비행기 몸체 충돌
	UBoxComponent* LeftWingCollision = nullptr;    // 왼쪽 날개 충돌
	UBoxComponent* RightWingCollision = nullptr;   // 오른쪽 날개 충돌

	float MovementSpeed = 200.0f; // 적 기본 이동 속도
};
