#include "UCubeComp.h"
#include "d3d11.h"

#include "Cube.h" // 헤더 추가하면 오류 발생, 작성 완료 되면 주석 해제

UCubeComp::UCubeComp()
	:Type(EPrimitiveType::Cube), IndexCount(sizeof(GCubeIndices) / sizeof(uint32)), Stride(sizeof(FVertexSimple)), Offset(0)
{

}

EPrimitiveType UCubeComp::GetPrimitiveType()
{
	return Type;
}

void UCubeComp::RenderPrimitive(ID3D11DeviceContext* deviceContext)
{
	UPrimitiveComponent::RenderPrimitive(deviceContext);

	deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
	deviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(IndexCount, 0, 0);
}
