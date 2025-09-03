#include "Background.h"

const string UBackground::BGSkySpriteAtlasKey;
const string UBackground::BGGroundSpriteAtlasKey;
const string UBackground::BGMountainSpriteAtlasKey;

UBackground::UBackground(UMeshRenderer* InRenderer, const string& atlasKey) : UObject(InRenderer)
{
	GetTransform()->SetScale(FVector3(0.1f, 0.1f, 0.1f));
	GetTransform()->SetLocation(FVector3(0, 0, 0));
}
UBackground::~UBackground()
{ 

}

// UObject override
FObjectType UBackground::GetType()
{ 
	return FObjectType::Background;
}
FVector3 UBackground::GetVelocity()
{
	return FVector3(0, 0, 0);
}
void UBackground::SetVelocity(const FVector3& newVelocity)
{ 
	
}
void UBackground::Update(float deltaTime)
{

}
UPhysicsComponent* UBackground::GetPhysicsComponent() const
{
	return nullptr;
}
