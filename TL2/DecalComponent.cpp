#include "pch.h"
#include "DecalComponent.h"

UDecalComponent::UDecalComponent() : OBB(DecalVolumeVertices, this) {}

UDecalComponent::~UDecalComponent()
{
    Material = nullptr;
}

void UDecalComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
    OBB.Render(Renderer, View, Proj);
}