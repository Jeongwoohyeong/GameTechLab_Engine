#include "BallTrail.h"
#include "App.h"

const string UBallTrail::BallTrailSpriteAtlasKey = "ball/ball_trail.png";
UBallTrail::UBallTrail(UMeshRenderer* InRenderer) : UObject(InRenderer)
{
    InRenderer->ChangeAtlasInfo(UApp::Ins->GetAtlasInfo(BallTrailSpriteAtlasKey));
    GetRenderer()->SetShader(UApp::Ins->SpriteAtlasInputLayout, UApp::Ins->SpriteAtlasVS, UApp::Ins->SpriteAtlasPS);

}

UBallTrail::~UBallTrail()
{
}

FObjectType UBallTrail::GetType()
{
    return FObjectType::ETC;
}

UPhysicsComponent* UBallTrail::GetPhysicsComponent() const
{
    return nullptr;
}

FVector3 UBallTrail::GetVelocity()
{
    return FVector3();
}

void UBallTrail::SetVelocity(const FVector3& newVelocity)
{
}

void UBallTrail::Update(float deltaTime)
{
    static float timeAccumulator = 0.0f;
    const float scaleInterval = 0.1f;

    timeAccumulator += deltaTime;

    if (timeAccumulator >= scaleInterval)
    {
        bShouldBeReleased = true;
    }
}
