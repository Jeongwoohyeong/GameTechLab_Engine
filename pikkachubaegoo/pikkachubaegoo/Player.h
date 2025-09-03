#pragma once

#include "Object.h"
#include "PhysicsComponent.h"

class UPlayer : public UObject
{
public:
	enum class PlayerState
	{
		Idle,
		Walking,
		Jumping,
		Sliding,
		Stunned,
		UpSpiking,
		Spiking,
		DownSpiking,
	};

private:
	static unsigned int playerCount;
	unsigned int playerIndex;
	// 상수값
	const float GRAVITY = 0.98f;
	const float MOVE_SPEED = 1.0f;
	const float SLIDE_SPEED = 2.0f;
	const float SLIDE_DURATION = 0.2f;
	const float STUN_DURATION = 0.1f;
	const float JUMP_STRENGTH = 1.1f;
	const float SPIKE_DURATION = 0.5f;

	// 상태 변수
	UPhysicsComponent* physicsComponent;
	PlayerState currentState = PlayerState::Idle;
	float slideTimer = 0.0f;
	float stunTimer = 0.0f;
	float spikeTimer = 0.0f;

	// 입력 플래그 (임시)
	bool isLeft = false;
	bool isRight = false;
	bool isJump = false;
	bool isSlide = false;
	bool isDown = false;
	bool isAction = false;

public:
	UPlayer(int newPlyaerIndex, UMeshRenderer* InRenderer);
	~UPlayer();

	// UObject 오버라이드
	FObjectType GetType() override;
	virtual class UPhysicsComponent* GetPhysicsComponent() const;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;

	void Update(float deltaTime) override;
	PlayerState GetState() const { return currentState; }

private:
	void SetInput();
};
