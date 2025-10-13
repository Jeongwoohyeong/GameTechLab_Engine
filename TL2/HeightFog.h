#pragma once

#include "Actor.h"

class UHeightFogComponent;

class AHeightFog : public AActor
{
public:
	DECLARE_CLASS(AHeightFog, AActor)

	void Initialize() override;

	UHeightFogComponent* GetHeightFogComponent() const { return HeightFogComponent; }
protected:
	~AHeightFog() override;
private:
	UHeightFogComponent* HeightFogComponent;
};