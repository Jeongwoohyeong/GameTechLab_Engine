#include "PhysicsComponent.h"
#include "math.h"
#include "stdlib.h"
#include "UPlayer.h"
#include "Define.h"

UPhysicsComponent::UPhysicsComponent(UObject* inOwner, const FRect& inCollider, const FRect& inBoundary, bool inIsGravity, float inGravityScale, bool inCanReflectWithWall)
	: owner(inOwner), velocity(), collider(inCollider), boundary(inBoundary), bIsGravity(inIsGravity), gravityScale(inGravityScale), bCanReflectWithWall(inCanReflectWithWall)
{
	collider.SetCenter(owner->GetTransform()->GetLocation());
}

void UPhysicsComponent::Update(float deltaTime)
{
	if (bIsGravity) ApplyGravity(deltaTime);
	UpdatePosition(deltaTime);
	CheckBoundaryCollision();
}

bool UPhysicsComponent::CheckCollision(const UPhysicsComponent* other)
{
	return collider.Intersects(other->GetColliderBounds());
}

void UPhysicsComponent::OnCollision(UPhysicsComponent* other)
{
	if (owner->GetType() != FObjectType::Ball)
	{
		return;
	}

	UObject* ball = GetOwner();
	UObject* otherObject = other->GetOwner();
	const FRect& otherCollider = other->GetColliderBounds();
	FVector2 overlap = CalculateOverlap(collider, otherCollider, ball->GetTransform()->GetLocation(), otherObject->GetTransform()->GetLocation());

	FVector3 newBallLocation = ball->GetTransform()->GetLocation();
	FVector3 newBallVelocity = GetVelocity();

	if (otherObject->GetType() == FObjectType::Player)
	{
		newBallLocation.y += fabs(overlap.y);
		UPlayer* player = static_cast<UPlayer*>(otherObject);
		if (player->IsSpiking())
		{
			newBallVelocity.x = (ball->GetTransform()->GetLocation().x - player->GetTransform()->GetLocation().x > 0) ? BALL_SPIKE_VELOCITY_X : -BALL_SPIKE_VELOCITY_X;
			newBallVelocity.y = BALL_SPIKE_VELOCITY_Y;
		}
		else
		{
			float relativeX = ball->GetTransform()->GetLocation().x - otherObject->GetTransform()->GetLocation().x;
			newBallVelocity.x = relativeX * 8.0f;

			if (newBallVelocity.x == 0)
			{
				int randomDirection = (rand() % 3) - 1;
				newBallVelocity.x = randomDirection * BALL_MIN_VELOCITY_X;
			}

			// Y Velocity
			if (fabs(newBallVelocity.y) > (BALL_MIN_VELOCITY_Y + BALL_SPIKE_VELOCITY_Y) / 2)
			{
				newBallVelocity.y = (BALL_MIN_VELOCITY_Y + BALL_SPIKE_VELOCITY_Y * 0.8f) / 2;
			}
			else if (fabs(newBallVelocity.y) < BALL_MIN_VELOCITY_Y)
			{
				newBallVelocity.y = BALL_MIN_VELOCITY_Y;
			}
			else
			{
				newBallVelocity.y = fabs(newBallVelocity.y);
			}
		}
	}
	else if (otherObject->GetType() == FObjectType::Wall)
	{
		if (fabs(overlap.x) < fabs(overlap.y))
		{
			newBallVelocity.x *= -1.0f;
			newBallLocation.x -= overlap.x;
		}
		else
		{
			newBallVelocity.y *= -1.0f;
			newBallLocation.y -= overlap.y;
		}

		if (newBallVelocity.x == 0)
		{
			int randomDirection = (rand() % 3) - 1;
			newBallVelocity.x = randomDirection * BALL_MIN_VELOCITY_X;
		}

	}

	// 최종 위치 및 속도 적용
	ball->GetTransform()->SetLocation(newBallLocation);
	SetVelocity(newBallVelocity);
}

FVector2 UPhysicsComponent::CalculateOverlap(const FRect& collider, const FRect& otherCollider, const FVector3& objectLocation, const FVector3& otherObjectLocation)
{
	float overlapX = 0.0f;
	if (collider._max.x > otherCollider._min.x && collider._min.x < otherCollider._max.x)
	{
		if (objectLocation.x > otherObjectLocation.x)
		{
			overlapX = collider._min.x - otherCollider._max.x;
		}
		else
		{
			overlapX = collider._max.x - otherCollider._min.x;
		}
	}

	float overlapY = 0.0f;
	if (collider._max.y > otherCollider._min.y && collider._min.y < otherCollider._max.y)
	{
		if (objectLocation.y > otherObjectLocation.y)
		{
			overlapY = collider._min.y - otherCollider._max.y;
		}
		else
		{
			overlapY = collider._max.y - otherCollider._min.y;
		}
	}

	return FVector2(overlapX, overlapY);
}

void UPhysicsComponent::ApplyGravity(float deltaTime)
{
	if (!IsGrounded()) velocity.y -= gravityScale * deltaTime;
}

void UPhysicsComponent::UpdatePosition(float deltaTime)
{
	FVector3 newLocation = owner->GetTransform()->GetLocation();
	newLocation.x += velocity.x * deltaTime;
	newLocation.y += velocity.y * deltaTime;

	owner->GetTransform()->SetLocation(newLocation);
	collider.SetCenter(owner->GetTransform()->GetLocation());
}

void UPhysicsComponent::CheckBoundaryCollision()
{
	FVector2 colliderMin = collider._min;
	FVector2 colliderMax = collider._max;

	if (colliderMin.x < boundary._min.x)
	{
		owner->GetTransform()->SetLocation(FVector2{ boundary._min.x + collider.GetWidth() / 2.0f, owner->GetTransform()->GetLocation().y });
		if (bCanReflectWithWall) velocity.x *= -1;
	}
	else if (colliderMax.x > boundary._max.x)
	{
		owner->GetTransform()->SetLocation(FVector2{ boundary._max.x - collider.GetWidth() / 2.0f, owner->GetTransform()->GetLocation().y });
		if (bCanReflectWithWall) velocity.x *= -1;
	}

	if (colliderMin.y < boundary._min.y)
	{
		owner->GetTransform()->SetLocation(FVector2{ owner->GetTransform()->GetLocation().x, boundary._min.y + collider.GetHeight() / 2.0f });
		if (bCanReflectWithWall) velocity.y *= -1;
		else if (bIsGravity) velocity.y = 0;
	}
	else if (colliderMax.y > boundary._max.y)
	{
		owner->GetTransform()->SetLocation(FVector2{ owner->GetTransform()->GetLocation().x, boundary._max.y - collider.GetHeight() / 2.0f });
		if (bCanReflectWithWall) velocity.y *= -1;
	}

	collider.SetCenter(owner->GetTransform()->GetLocation());
}

bool UPhysicsComponent::IsGrounded(float groundEpsilon) const
{
	if (!bIsGravity) return false;

	return (fabs(collider._min.y - boundary._min.y) < groundEpsilon);
}
