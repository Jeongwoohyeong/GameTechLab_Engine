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
void UPrimitiveComponent::CreateAABB()
{
	FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	const uint32 VertexCount = VertexByteWidth / VertexStride;
	for (int Offset = 0; Offset < VertexByteWidth; Offset += VertexStride)
	{
		const char* Ptr = static_cast<const char*>(Vertices) + Offset;
		float X = *reinterpret_cast<const float*>(Ptr);
		float Y = *reinterpret_cast<const float*>(Ptr + sizeof(float));
		float Z = *reinterpret_cast<const float*>(Ptr + 2 * sizeof(float));
		
		if (Min.X > X) Min.X = X;
		if (Min.Y > Y) Min.Y = Y;
		if (Min.Z > Z) Min.Z = Z;
		if (Max.X < X) Max.X = X;
		if (Max.Y < Y) Max.Y = Y;
		if (Max.Z < Z) Max.Z = Z;
	}

	AABB.Min = Min;
	AABB.Max = Max;
}
