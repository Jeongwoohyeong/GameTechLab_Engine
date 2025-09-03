#include "RenderContext.h"

void URenderContext::Draw(UMesh* mesh, const FVector3& location)
{
	D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
	deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, & constantbufferMSR);
	FVector3* constants = (FVector3*)constantbufferMSR.pData;
	{
		constants->x = location.x;
		constants->y = location.y;
		constants->z = location.z;
	}
	deviceContext->Unmap(constantBuffer, 0);

    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &offset);
    deviceContext->IASetIndexBuffer(mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, offset);
    deviceContext->DrawIndexed(mesh->IndexCount, 0, 0);
}
