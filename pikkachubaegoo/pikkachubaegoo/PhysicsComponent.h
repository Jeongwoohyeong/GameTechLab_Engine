#pragma once
#include "Object.h"
#include "Rect.h"
#include "Vector.h"

class UPhysicsComponent
{
public:
	UPhysicsComponent(UObject* inOwner, const FRect& inCollider, const FRect& inBoundary,
        bool inIsGravity = false, float inGravityScale = 0.f, bool inCanReflectWithWall = false);

    FVector GetVelocity() const { return velocity; }
    void SetVelocity(const FVector& inVelocity) { velocity = inVelocity; }

	void Update(float deltaTime);
	bool CheckCollision(const UPhysicsComponent* other);
	void OnCollision(UPhysicsComponent* other);

    UObject* GetOwner() const { return owner; }
    const FRect& GetColliderBounds() const { return collider; }

private:
    void ApplyGravity(float deltaTime);
    void UpdatePosition(float deltaTime);
    void CheckBoundaryCollision();

    UObject* owner;
    FVector velocity;
    FRect collider;
    FRect boundary;


// Gravity Section
public:
    void SetIsGravity(bool inIsGravity) 
        { bIsGravity = inIsGravity; }
    void SetGravityScale(float inGravityScale) 
        { gravityScale = inGravityScale; }
    void SetCanReflectWithWall(bool inCanReflectWithWall) 
        { bCanReflectWithWall = inCanReflectWithWall; }

private:
    bool bIsGravity;
    float gravityScale;
    bool bCanReflectWithWall;
};

