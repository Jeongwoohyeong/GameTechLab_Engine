#pragma once

#include "Object.h"
#include "ImGui/imgui_impl_dx11.h"
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
		Spiking
	};

private:
	// 상수값
	const float GRAVITY = 0.000098f;
	const float GROUND_LEVEL = 0;
	const float WALK_SPEED = 1;
	const float MOVE_SPEED = 0.001f;
	const float SLIDE_SPEED = 5.0f;
	const float SLIDE_DURATION = 1.0f;
	const float STUN_DURATION = 0.5f;
	const float JUMP_STRENGTH = 0.01f;
	const float SPIKE_DURATION = 0.5f;

	// 상태 변수
	UPhysicsComponent* physicsComponent;
	PlayerState currentState = PlayerState::Idle;
	float slideTimer = 0.0f;
	float stunTimer = 0.0f;
	float spikeTimer = 0.0f;
	bool isSpiking = false;

	// 위치, 속도, 크기
	FVector3 location;
	float size;

	// 입력 플래그 (임시)
	bool isLeft = false;
	bool isRight = false;
	bool isJump = false;
	bool isAction = false;

public:
	/*ID3D11Buffer* VertexBuffer;
	UINT NumVertices;*/

	UPlayer();

	// UObject 오버라이드
	FObjectType GetType() override;
	FVector3 GetLocation() override;
	void SetLocation(const FVector3& newLocation) override;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;
	float GetRadius() override;
	float GetMass() override;

	void Update(float deltaTime) override;

	// 입력 제어 함수?
	void SetInput(bool left, bool right, bool jump, bool action);
};
