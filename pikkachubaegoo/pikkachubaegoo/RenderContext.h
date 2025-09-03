#pragma once
#include "D3DUtil.h"
#include "Mesh.h"
class URenderContext {
public:
    URenderContext(ID3D11DeviceContext* context, ID3D11Buffer* cBuffer) : deviceContext(context), constantBuffer(cBuffer){}

    void Draw(UMesh* mesh, const FVector3& location);
private:
    ID3D11DeviceContext* deviceContext;
    ID3D11Buffer* constantBuffer;
};
