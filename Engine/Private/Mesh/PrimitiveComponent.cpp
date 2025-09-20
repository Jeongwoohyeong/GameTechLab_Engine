#include "pch.h"
#include "Mesh/PrimitiveComponent.h"
#include "Manager/ResourceManager.h"

#include <algorithm>

//IMPLEMENT_ABSTRACT_CLASS(UPrimitiveComponent, USceneComponent)

IMPLEMENT_CLASS(UPrimitiveComponent, USceneComponent)

UPrimitiveComponent::UPrimitiveComponent()
{
	ComponentType = EComponentType::Primitive;
}


const TArray<FVertex>* UPrimitiveComponent::GetVerticesData() const
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	return ResourceManager.GetVertexData(Type);
}

const TArray<FVertex>* UPrimitiveComponent::GetReducedVerticesData() const
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	return ResourceManager.GetReducedVertexData(Type);
}

const TArray<uint32>* UPrimitiveComponent::GetIndicesData() const
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	return ResourceManager.GetIndexData(Type);
}

void UPrimitiveComponent::SetTopology(D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
	Topology = InTopology;
}

D3D11_PRIMITIVE_TOPOLOGY UPrimitiveComponent::GetTopology() const
{
	return Topology;
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

//FAABB UPrimitiveComponent::GetWorldBounds() const
//{
//	return GetLocalBounds();
//}



/*
* 리소스는 Manager가 관리하고 component는 참조만 함.
*
*/

IMPLEMENT_CLASS(USphereComponent, UPrimitiveComponent)
USphereComponent::USphereComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Sphere;

	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB USphereComponent::GetWorldBounds() const
{
	// 실제 버텍스 데이터에서 바운딩 박스 계산
	return UPrimitiveComponent::GetWorldBounds();
}

IMPLEMENT_CLASS(UCubeComponent, UPrimitiveComponent)
UCubeComponent::UCubeComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Cube;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB UCubeComponent::GetWorldBounds() const
{
	// 실제 버텍스 데이터에서 바운딩 박스 계산
	return UPrimitiveComponent::GetWorldBounds();
}


IMPLEMENT_CLASS(UTriangleComponent, UPrimitiveComponent)
UTriangleComponent::UTriangleComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Triangle;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB UTriangleComponent::GetWorldBounds() const
{
	return UPrimitiveComponent::GetWorldBounds();
}

IMPLEMENT_CLASS(USquareComponent, UPrimitiveComponent)
USquareComponent::USquareComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Square;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB USquareComponent::GetWorldBounds() const
{
	return UPrimitiveComponent::GetWorldBounds();
}
