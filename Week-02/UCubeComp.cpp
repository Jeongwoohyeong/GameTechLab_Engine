#include "UCubeComp.h"
#include "d3d11.h"
#include "URenderer.h"
#include "Cube.h"

UCubeComp::UCubeComp()
	:Type(EPrimitiveType::Cube)
{
	UPrimitiveComponent::VertexBuffer = URenderer::CubeVertexBuffer;
	UPrimitiveComponent::IndexBuffer = URenderer::CubeIndexBuffer;
	UPrimitiveComponent::Offset = 0;
	UPrimitiveComponent::Stride = sizeof(FVertexSimple);
	UPrimitiveComponent::Vertices = &GCubeVertices;
	UPrimitiveComponent::Indices = &GCubeIndices;
	UPrimitiveComponent::VertexByteWidth = sizeof(GCubeVertices);
	UPrimitiveComponent::IndexByteWidth = sizeof(GCubeIndices);
	UPrimitiveComponent::IndexCount = IndexByteWidth / sizeof(uint32);
}

EPrimitiveType UCubeComp::GetPrimitiveType()
{
	return Type;
}

void UCubeComp::RenderPrimitive(ID3D11DeviceContext* deviceContext)
{
	UPrimitiveComponent::RenderPrimitive(deviceContext);	
}
