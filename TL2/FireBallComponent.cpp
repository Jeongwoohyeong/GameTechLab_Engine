#include "pch.h"
#include "FireBallComponent.h"
UFireBallComponent::UFireBallComponent() : Intensity(10.f), Radius(50.f), RadiusFallOff(2.f), Color(FLinearColor(1.f, .5f, .1f, 1.f))
{
	//Super_t::SetMaterial("FireBallShader.hlsl");
}

void UFireBallComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	FVector4 WorldLocation(GetWorldLocation().X, GetWorldLocation().Y, GetWorldLocation().Z, 1.0f);
	float InvRadius = Radius > 0.f ? 1.f / Radius : 0.f;
	FVector4 Parameters(Intensity, Radius, InvRadius, RadiusFallOff);
	FireBallBufferType FireBallData = { WorldLocation, Color, Parameters };

	Renderer->AddFireBallToScene(FireBallData);
}

UObject* UFireBallComponent::Duplicate()
{
	return nullptr;
}

void UFireBallComponent::DuplicateSubObjects()
{
}
