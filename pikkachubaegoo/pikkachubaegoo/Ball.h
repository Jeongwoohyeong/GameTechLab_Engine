#pragma once
#include "Object.h"

class UBall : public UObject
{
public:
	UBall(UMeshRenderer* InRenderer);
	~UBall();

	// UObject override
	FObjectType GetType() override;
	FVector3 GetLocation() override;
	void SetLocation(const FVector3& newLocation) override;
	virtual class UPhysicsComponent* GetPhysicsComponent() const;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;
	void Update(float deltaTime) override;

private:
	// Cnst Value
	const float GRAVITY = 0.98f * 2.0f;
	const float GROUND_LEVEL = -1;
	const float WALK_SPEED = 1;
	const float MOVE_SPEED = 0.1f;
	const float SLIDE_SPEED = 1.0f;
	const float SLIDE_DURATION = 0.5f;
	const float STUN_DURATION = 0.2f;
	const float JUMP_STRENGTH = 1.0f;
	const float SPIKE_DURATION = 0.5f;

	// Component
	class UPhysicsComponent* physicsComponent;

	// Object
	FVector3 location;
	float size;
};
