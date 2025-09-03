#pragma once
#include "Vector.h"
#include "UMeshRenderer.h"
enum FObjectType
{
	Player,
	Ball,
	Wall,
};

class UObject
{
private:
	UMeshRenderer* renderer = nullptr;
public:
	UObject(UMeshRenderer* InRenderer) : renderer(InRenderer) {}
	virtual ~UObject();

	virtual FObjectType GetType() = 0;

	virtual FVector3 GetLocation() = 0;
	virtual void SetLocation(const FVector3& newLocation) = 0;

	virtual class UPhysicsComponent* GetPhysicsComponent() const = 0;
	virtual FVector3 GetVelocity() = 0;
	virtual void SetVelocity(const FVector3& newVelocity) = 0;

	virtual void Update(float deltaTime) = 0;
	void Draw();
};
