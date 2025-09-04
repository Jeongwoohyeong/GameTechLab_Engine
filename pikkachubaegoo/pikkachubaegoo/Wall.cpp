#include "Wall.h"
#include "PhysicsComponent.h"
#include "Define.h"
#include "App.h"
const string UWall::WallSpriteAtlasKey = "objects/net_pillar.png";

UWall::UWall(UMeshRenderer* InRenderer) : UObject(InRenderer)
{
	InRenderer->ChangeAtlasInfo(UApp::Ins->GetAtlasInfo(WallSpriteAtlasKey));

	FRect collider(GetTransform()->GetLocation(), 0.05f, 0.5f);
	FRect boundary(FVector2(-1.0f, GROUND_LEVEL), FVector2(1.0f, 1.0f));
	physicsComponent = new UPhysicsComponent(this, collider, boundary, false);
	GetRenderer()->SetShader(UApp::Ins->SpriteAtlasInputLayout, UApp::Ins->SpriteAtlasVS, UApp::Ins->SpriteAtlasPS);
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
