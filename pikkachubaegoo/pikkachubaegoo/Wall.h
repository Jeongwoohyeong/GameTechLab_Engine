#pragma once
#include "Object.h"

class UWall : public UObject
{
public:
	UWall(UMeshRenderer* InRenderer);
	~UWall();

	// UObject override
	FObjectType GetType() override;
	virtual class UPhysicsComponent* GetPhysicsComponent() const;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;
	void Update(float deltaTime) override;

private:
	const float GROUND_LEVEL = -1;

	// Component
	class UPhysicsComponent* physicsComponent;

	// Object
	float size;
};
