#include "UPrimitiveComponent.h"
#include "d3d11.h"

UPrimitiveComponent::UPrimitiveComponent()
	:Transform(FTransform())
{
}

void UPrimitiveComponent::RenderPrimitive(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
	deviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(IndexCount, 0, 0);
}

void UPrimitiveComponent::Release()
{
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
}