#include "Punch.h"
#include "App.h"
#include "Define.h"

const string UPunch::PunchSpriteAtlasKey = "ball/ball_punch.png";
UPunch::UPunch(UMeshRenderer* InRenderer) : UObject(InRenderer)
{
	InRenderer->ChangeAtlasInfo(UApp::Ins->GetAtlasInfo(PunchSpriteAtlasKey));
}

UPunch::~UPunch()
{
}

FObjectType UPunch::GetType()
{
	return FObjectType::ETC;
}

UPhysicsComponent* UPunch::GetPhysicsComponent() const
{
	return nullptr;
}

FVector3 UPunch::GetVelocity()
{
	return FVector3();
}

void UPunch::SetVelocity(const FVector3& newVelocity)
{
	return;
}

void UPunch::Update(float deltaTime)
{
    static float timeAccumulator = 0.0f;
    const float scaleInterval = 0.1f;

    timeAccumulator += deltaTime;

    while (timeAccumulator >= scaleInterval)
    {
        const FVector3& Scale = GetTransform()->GetScale();
        GetTransform()->SetScale(Scale * PUNCH_SCALE_SPEED);

        timeAccumulator -= scaleInterval;

        // 크기가 너무 작아지면 오브젝트 해제
        if (GetTransform()->GetScale().x < 0.02f)
        {
            bShouldBeReleased = true;
            
        }
    }
}
