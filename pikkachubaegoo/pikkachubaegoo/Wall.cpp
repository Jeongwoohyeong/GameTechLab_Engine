#include "Wall.h"
#include "PhysicsComponent.h"

UWall::UWall(UMeshRenderer* InRenderer) : UObject(InRenderer)
{
	SetLocation(FVector3(0, GROUND_LEVEL, 0));
	FRect collider(FVector3(-size, -size, 1), FVector3(size, size, 1));
	FRect boundary(FVector3(-1.0f, GROUND_LEVEL, 0), FVector3(1.0f, 1.0f, 0));
	physicsComponent = new UPhysicsComponent(this, collider, boundary, false);
}

UWall::~UWall()
{
	if (physicsComponent)
	{
		delete physicsComponent;
		physicsComponent = nullptr;
	}
}

FObjectType UWall::GetType()
{
	return FObjectType::Wall;
}

UPhysicsComponent* UWall::GetPhysicsComponent() const
{
	return physicsComponent;
}

FVector3 UWall::GetVelocity()
{
	return FVector3();
}

void UWall::SetVelocity(const FVector3& newVelocity)
{
}

void UWall::Update(float deltaTime)
{
	physicsComponent->Update(deltaTime);
}
