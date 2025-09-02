#include "UPlayer.h"
#include <iostream>

// --- UObject overrides ---

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

FVector3 UPlayer::GetVelocity()
{
    return velocity;
}

void UPlayer::SetVelocity(const FVector3& newVelocity)
{
    velocity = newVelocity;
}

float UPlayer::GetRadius()
{
    return 0.0f;
}

float UPlayer::GetMass()
{
    return 0.0f;
}

// --- 입력 설정 함수 (선택적) ---
void UPlayer::SetInput(bool left, bool right, bool jump, bool action)
{
    isLeft = left;
    isRight = right;
    isJump = jump;
    isAction = action;
}

// --- Update 로직 ---
void UPlayer::Update(float deltaTime)
{
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
        if (isGrounded)
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

    // --- 중력 ---
    if (!isGrounded)
    {
        velocity.y -= GRAVITY * deltaTime;
    }

    // --- 위치 업데이트 ---
    location = location + (velocity * deltaTime);

    // --- 지면 충돌 처리 ---
    if (location.y <= GROUND_LEVEL)
    {
        location.y = GROUND_LEVEL;
        if (velocity.y < 0)
        {
            velocity.y = 0;
        }
        isGrounded = true;
    }
    else
    {
        isGrounded = false;
    }
}
