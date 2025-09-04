#pragma once
#include "Object.h"
#include <string>
using namespace std;

class UBall : public UObject
{
public:	
	const static string BallSpriteAtlasKey;

public:
	UBall(UMeshRenderer* InRenderer);
	~UBall();

	// UObject override
	FObjectType GetType() override;
	virtual class UPhysicsComponent* GetPhysicsComponent() const;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;
	void Update(float deltaTime) override;
	void Reset() override;

	bool IsSpiking() const { return bIsSpiking; }
	void SetIsSpiking(bool IsSpiking);

private:
	// Const Value
	const float GRAVITY = 0.98f * 2.0f;

	// Component
	class UPhysicsComponent* physicsComponent;

	// Object
	FVector3 location;
	float size;

	UObject* prevTrail = nullptr;
	FVector3 prevLocation;
	bool bIsSpiking = false;
};
