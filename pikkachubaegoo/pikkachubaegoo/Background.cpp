#include "Background.h"
#include "App.h"
const string UBackground::BGSkySpriteAtlasKey = "objects/sky_blue.png";
const string UBackground::BGGroundSpriteAtlasKey = "objects/ground_yellow.png";
const string UBackground::BGMountainSpriteAtlasKey = "objects/mountain.png";

UBackground::UBackground(UMeshRenderer* InRenderer, const string& atlasKey, const FVector3& location, const FVector3& scale) : UObject(InRenderer)
{
	InRenderer->ChangeAtlasInfo(UApp::Ins->GetAtlasInfo(atlasKey));
	GetTransform()->SetLocation(location);
	GetTransform()->SetScale(scale);
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
