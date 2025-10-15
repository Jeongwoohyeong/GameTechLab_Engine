#pragma once

#include "Actor.h"

class UHeightFogComponent;

class AHeightFog : public AActor
{
public:
	DECLARE_CLASS(AHeightFog, AActor)

	void Initialize() override;

	UHeightFogComponent* GetHeightFogComponent() const { return HeightFogComponent; }
	void SetHeightFogComponent(UHeightFogComponent* InHeightFogComponent) { HeightFogComponent = InHeightFogComponent; }

	// PIE 복제
	UObject* Duplicate() override;
	void DuplicateSubObjects() override;

protected:
	~AHeightFog() override;

private:
	UHeightFogComponent* HeightFogComponent = nullptr;
};