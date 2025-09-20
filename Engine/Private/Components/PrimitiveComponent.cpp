#include "pch.h"
#include "Components/PrimitiveComponent.h"
#include "Manager/ResourceManager.h"

#include <algorithm>

IMPLEMENT_ABSTRACT_CLASS(UPrimitiveComponent, USceneComponent)

//IMPLEMENT_CLASS(UPrimitiveComponent, USceneComponent)

UPrimitiveComponent::UPrimitiveComponent()
{
	ComponentType = EComponentType::Primitive;
}

