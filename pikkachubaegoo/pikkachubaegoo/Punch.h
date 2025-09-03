#pragma once
#include "Object.h"
#include <string>
using namespace std;

class UPunch : public UObject
{
public:
	const static string PunchSpriteAtlasKey;
public:
	UPunch(UMeshRenderer* InRenderer);
	~UPunch();

	// UObject override
	FObjectType GetType() override;
	virtual class UPhysicsComponent* GetPhysicsComponent() const;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;
	void Update(float deltaTime) override;
};
