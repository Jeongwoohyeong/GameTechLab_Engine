#pragma once
#include "D3DUtil.h"
#include "Mesh.h"
class URenderContext {
public:
    URenderContext(ID3D11DeviceContext* context) : deviceContext(context) {}

    void Draw(UMesh* mesh);
private:
    ID3D11DeviceContext* deviceContext;
};