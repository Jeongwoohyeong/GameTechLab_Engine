#include "RenderContext.h"

void URenderContext::Draw(UMesh* mesh)
{
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &offset);
    deviceContext->IASetIndexBuffer(mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, offset);
    deviceContext->DrawIndexed(mesh->IndexCount, 0, 0);
}
