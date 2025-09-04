#include "Background.h"
#include "App.h"

UBackground::UBackground(UMeshRenderer* InRenderer,  const FVector3& location, const FVector3& scale) : UObject(InRenderer)
{
	GetTransform()->SetLocation(location);
	GetTransform()->SetScale(scale);
	GetRenderer()->ChangeAtlasInfo(FVector4(0, 0, 1024, 1024));
	GetRenderer()->SetShader(UApp::Ins->SpriteInputLayout, UApp::Ins->SpriteVS, UApp::Ins->SpritePS);

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
