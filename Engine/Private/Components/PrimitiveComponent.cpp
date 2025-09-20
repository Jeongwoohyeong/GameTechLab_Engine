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


const TArray<FVertex>* UPrimitiveComponent::GetVerticesData() const
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	return ResourceManager.GetVertexData(Type);
}

FAABB UPrimitiveComponent::GetWorldBounds() const
{
	if (!Vertices || Vertices->empty())
	{
		return FAABB();
	}

	FAABB Bounds;
	const FMatrix& Transform = GetWorldTransformMatrix();

	for (const FVertex& Vertex : *Vertices)
	{
		FVector4 TransformedPoint = FVector4(Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z, 1.0f) * Transform;
		Bounds.AddPoint(FVector(TransformedPoint.X, TransformedPoint.Y, TransformedPoint.Z));
	}
	return Bounds;
}
