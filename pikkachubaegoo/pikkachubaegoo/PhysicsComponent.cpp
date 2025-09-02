#include "PhysicsComponent.h"
#include "math.h"

UPhysicsComponent::UPhysicsComponent(UObject* inOwner, const FRect& inCollider, const FRect& inBoundary, bool inIsGravity, float inGravityScale, bool inCanReflectWithWall)
	: owner(inOwner), velocity(), collider(inCollider), boundary(inBoundary), bIsGravity(inIsGravity), gravityScale(inGravityScale), bCanReflectWithWall(inCanReflectWithWall)
{
	
}

void UPhysicsComponent::Update(float deltaTime)
{
	if(bIsGravity) ApplyGravity(deltaTime);
	UpdatePosition(deltaTime);
	CheckBoundaryCollision();
}

bool UPhysicsComponent::CheckCollision(const UPhysicsComponent* other)
{
	return collider.Intersects(other->GetColliderBounds());
}

void UPhysicsComponent::OnCollision(UPhysicsComponent* other)
{
	if ((owner->GetType() == FObjectType::Ball && other->GetOwner()->GetType() == FObjectType::Player))
	{
		UObject* ball = GetOwner();
		UObject* player = other->GetOwner();

		FVector3 newBallVelocity = GetVelocity();

		// X Velocity
		float relativeX = ball->GetLocation().x - player->GetLocation().x;
		if (relativeX > 0)
		{
			newBallVelocity.x = fabs(newBallVelocity.x);
		}
		else if (relativeX < 0)
		{
			newBallVelocity.x = -fabs(newBallVelocity.x);
		}

		// Y Velocity
		constexpr float MIN_VELOCITY_Y = 15.0f;
		float currentVelocityY = -fabs(newBallVelocity.y);

		if (fabs(currentVelocityY) < MIN_VELOCITY_Y)
		{
			newBallVelocity.y = -MIN_VELOCITY_Y;
		}
		else
		{
			newBallVelocity.y = currentVelocityY;
		}

		SetVelocity(newBallVelocity);
	}
}

void UPhysicsComponent::ApplyGravity(float deltaTime)
{
	velocity.y -= gravityScale * deltaTime;
}

void UPhysicsComponent::UpdatePosition(float deltaTime)
{
	FVector3 newLocation = owner->GetLocation();
	newLocation.x += velocity.x * deltaTime;
	newLocation.y += velocity.y * deltaTime;

	owner->SetLocation(newLocation);
}

void UPhysicsComponent::CheckBoundaryCollision()
{
	FVector3 colliderMin = collider.min;
	FVector3 colliderMax = collider.max;

	if (colliderMin.x < boundary.min.x)
	{
		owner->SetLocation({ boundary.min.x + collider.GetWidth() / 2.0f, owner->GetLocation().y });
		if (bCanReflectWithWall) velocity.x *= -1;
	}
	else if (colliderMax.x > boundary.max.x)
	{
		owner->SetLocation({ boundary.max.x - collider.GetWidth() / 2.0f, owner->GetLocation().y });
		if (bCanReflectWithWall) velocity.x *= -1;
	}

	if (colliderMin.y < boundary.min.y)
	{
		owner->SetLocation({ owner->GetLocation().x, boundary.min.y + collider.GetHeight() / 2.0f });
		if (bCanReflectWithWall) velocity.y *= -1;
		else if (bIsGravity) velocity.y = 0;
	}
	else if (colliderMax.y > boundary.max.y)
	{
		owner->SetLocation({ owner->GetLocation().x, boundary.max.y - collider.GetHeight() / 2.0f });
		if (bCanReflectWithWall) velocity.y *= -1;
	}

	collider.SetCenter(owner->GetLocation());
}
