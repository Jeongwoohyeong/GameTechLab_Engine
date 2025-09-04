#include "Ball.h"
#include "Rect.h"
#include "PhysicsComponent.h"
#include "Define.h"
#include "App.h"
#include "ObjectFactory.h"
#include "BallTrail.h"
const string UBall::BallSpriteAtlasKey = "ball/ball_0.png";

UBall::UBall(UMeshRenderer* InRenderer) : UObject(InRenderer)
{
	InRenderer->ChangeAtlasInfo(UApp::Ins->GetAtlasInfo(BallSpriteAtlasKey));
	size = 0.1f;
	location = FVector3(0, GROUND_LEVEL, 0);
	FRect collider(FVector3(-size, -size, 1), FVector3(size, size, 1));
	FRect boundary(FVector3(-1.0f, GROUND_LEVEL, 0), FVector3(1.0f, 1.0f, 0));
	physicsComponent = new UPhysicsComponent(this, collider, boundary, true, GRAVITY, true);
	GetRenderer()->SetShader(UApp::Ins->SpriteAtlasInputLayout, UApp::Ins->SpriteAtlasVS, UApp::Ins->SpriteAtlasPS);

}

UBall::~UBall()
{
	if (physicsComponent)
	{
		delete physicsComponent;
		physicsComponent = nullptr;
	}
}

FObjectType UBall::GetType()
{
	return FObjectType::Ball;
}

UPhysicsComponent* UBall::GetPhysicsComponent() const
{
	return physicsComponent;
}

FVector3 UBall::GetVelocity()
{
	if(physicsComponent) return physicsComponent->GetVelocity();
	return FVector3();
}

void UBall::SetVelocity(const FVector3& newVelocity)
{
	return physicsComponent->SetVelocity(newVelocity);
}

void UBall::Update(float deltaTime)
{
	physicsComponent->Update(deltaTime);
	GetTransform()->AddRotationZ(GetVelocity().x * BALL_ROTATE_RATIO * deltaTime);

	static float timeAccumulator = 0.0f;
	const float executionInterval = 0.03f;
	timeAccumulator += deltaTime;
	if (timeAccumulator >= executionInterval)
	{
		if (IsSpiking())
		{
			if(!(prevLocation.x == 0 && prevLocation.y == 0))
				prevTrail = UObjectFactory::GetInstance()->CreateBallTrail(prevLocation, FVector3(size, size));
		}
		prevLocation = GetTransform()->GetLocation();
		timeAccumulator -= executionInterval;
	}
}

void UBall::Reset()
{
	UObject::Reset();
	SetIsSpiking(false);
	if (prevTrail)
	{
		prevTrail->bShouldBeReleased = true;
	}
}
