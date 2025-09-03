#pragma once
#include "Vector.h"
#include "UMeshRenderer.h"
#include "Transform.h"

enum FObjectType
{
	Player,
	Ball,
	Wall,
};

class UObject
{
private:
	Transform transform;
	UMeshRenderer* renderer = nullptr;
public:
	UObject(UMeshRenderer* InRenderer) : renderer(InRenderer) {}
	virtual ~UObject();

	virtual FObjectType GetType() = 0;

	virtual class UPhysicsComponent* GetPhysicsComponent() const = 0;
	Transform* GetTransform();
	virtual FVector3 GetVelocity() = 0;
	virtual void SetVelocity(const FVector3& newVelocity) = 0;

	virtual void Update(float deltaTime) = 0;
	void Draw();
};
