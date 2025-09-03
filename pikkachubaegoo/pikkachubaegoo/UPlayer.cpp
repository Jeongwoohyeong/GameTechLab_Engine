#include "UPlayer.h"
#include "inputclass.h"
#include "Windows.h"
#include <iostream>

unsigned int UPlayer::playerCount = 0;


UPlayer::UPlayer(UMeshRenderer* InRenderer) : UObject(InRenderer)
{
	playerIndex = playerCount++;
	size = 0.1f;
	location = FVector3(0, GROUND_LEVEL, 0);
	FRect collider(FVector3(-size / 2, -size, 1), FVector3(size, size, 1)); // 플레이어의 충돌 박스 설정
	FRect boundary(FVector3(-100.0f, GROUND_LEVEL, 0), FVector3(200.0f, 200.0f, 0)); // 경계 설정
	physicsComponent = new UPhysicsComponent(this, collider, boundary, true, GRAVITY);
}

UPlayer::~UPlayer()
{
	if (physicsComponent)
	{
		delete physicsComponent;
		physicsComponent = nullptr;
	}
}

FObjectType UPlayer::GetType()
{
	return FObjectType::Player;
}

FVector3 UPlayer::GetLocation()
{
	return location;
}

void UPlayer::SetLocation(const FVector3& newLocation)
{
	location = newLocation;
}

UPhysicsComponent* UPlayer::GetPhysicsComponent() const
{
	return physicsComponent;
}

FVector3 UPlayer::GetVelocity()
{
	return physicsComponent->GetVelocity();
}

void UPlayer::SetVelocity(const FVector3& newVelocity)
{
	physicsComponent->SetVelocity(newVelocity);
}

// --- 입력 설정 함수 (임시) ---
void UPlayer::SetInput()
{
	if (playerIndex == 0)
	{
		isLeft = UInput::GetInstance()->IsKeyDown(VK_LEFT);
		isRight = UInput::GetInstance()->IsKeyDown(VK_RIGHT);
		isJump = UInput::GetInstance()->IsKeyPressed(VK_UP);
		isAction = UInput::GetInstance()->IsKeyPressed(VK_DOWN);
	}
	else if (playerIndex == 1)
	{
		isLeft = UInput::GetInstance()->IsKeyDown('A');
		isRight = UInput::GetInstance()->IsKeyDown('D');
		isJump = UInput::GetInstance()->IsKeyPressed('W');
		isAction = UInput::GetInstance()->IsKeyPressed('S');
	}
}

// --- Update 로직 ---
void UPlayer::Update(float deltaTime)
{
	SetInput();

	FVector3 velocity = physicsComponent->GetVelocity();
	float horizontalInput = 0.0f;
	if (isLeft) horizontalInput = -1.0f;
	else if (isRight) horizontalInput = 1.0f;
	bool jumpPressed = isJump;
	bool slidePressed = isAction;

	switch (currentState)
	{
	case PlayerState::Idle:
		if (jumpPressed)
		{
			velocity.y = JUMP_STRENGTH;
			currentState = PlayerState::Jumping;
		}
		else if (horizontalInput != 0)
		{
			currentState = PlayerState::Walking;
		}
		break;

	case PlayerState::Walking:
		if (jumpPressed)
		{
			velocity.y = JUMP_STRENGTH;
			currentState = PlayerState::Jumping;
		}
		else if (horizontalInput == 0)
		{
			currentState = PlayerState::Idle;
		}
		else if (slidePressed)
		{
			currentState = PlayerState::Sliding;
			slideTimer = SLIDE_DURATION;
		}
		break;

	case PlayerState::Jumping:
		if (physicsComponent->IsGrounded())
		{
			currentState = PlayerState::Idle;
		}
		else if (slidePressed)
		{
			currentState = PlayerState::Spiking;
			spikeTimer = SPIKE_DURATION;
			isSpiking = true;
		}
		break;

	case PlayerState::Spiking:
		spikeTimer -= deltaTime;
		if (spikeTimer <= 0)
		{
			isSpiking = false;
			currentState = PlayerState::Jumping;
		}
		break;

	case PlayerState::Sliding:
		slideTimer -= deltaTime;
		if (slideTimer <= 0)
		{
			currentState = PlayerState::Stunned;
			stunTimer = STUN_DURATION;
		}
		break;

	case PlayerState::Stunned:
		stunTimer -= deltaTime;
		if (stunTimer <= 0)
		{
			currentState = PlayerState::Idle;
		}
		break;
	}

	// --- 수평 속도 설정 ---
	switch (currentState)
	{
	case PlayerState::Idle:
	case PlayerState::Stunned:
		velocity.x = 0;
		break;
	case PlayerState::Walking:
	case PlayerState::Jumping:
	case PlayerState::Spiking:
		velocity.x = horizontalInput * MOVE_SPEED;
		break;
	case PlayerState::Sliding:
		velocity.x = (velocity.x > 0) ? SLIDE_SPEED : -SLIDE_SPEED;
		break;
	}

	physicsComponent->SetVelocity(velocity);
	// 위치 업데이트
	physicsComponent->Update(deltaTime);
}
