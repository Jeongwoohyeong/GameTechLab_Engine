#pragma once
#include "Object.h"
#include <string>
using namespace std;

class UWall : public UObject
{
public:
	const static string WallSpriteAtlasKey;
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
	// Component
	class UPhysicsComponent* physicsComponent;

	// Object
	FVector2 colliderExtent = FVector2(0.1f, 0.3f);
};
