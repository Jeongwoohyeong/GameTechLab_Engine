#include "UCubeComp.h"
#include "d3d11.h"

#include "Cube.h" // 헤더 추가하면 오류 발생, 작성 완료 되면 주석 해제

UCubeComp::UCubeComp()
	:Type(EPrimitiveType::Cube)
{
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
