#pragma once
#include "Object.h"
#include <string>

class UBackground : public UObject
{
public:
	const static string BGSkySpriteAtlasKey;
	const static string BGGroundSpriteAtlasKey;
	const static string BGMountainSpriteAtlasKey;
public:
	UBackground(UMeshRenderer* InRenderer, const string& atlasKey, const FVector3& location, const FVector3& scale);
	~UBackground();

	// UObject override
	FObjectType GetType() override;
	FVector3 GetVelocity() override;
	void SetVelocity(const FVector3& newVelocity) override;
	void Update(float deltaTime) override;
	virtual class UPhysicsComponent* GetPhysicsComponent() const;
};