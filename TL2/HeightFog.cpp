#include "pch.h"
#include "HeightFog.h"
#include "HeightFogComponent.h"

//IMPLEMENT_CLASS(AHeightFog)

void AHeightFog::Initialize()
{
	Name = "Height Fog";
	HeightFogComponent = CreateDefaultSubobject<UHeightFogComponent>("HeightFogComponent");
	RootComponent = HeightFogComponent;
}

AHeightFog::~AHeightFog()
{
}
