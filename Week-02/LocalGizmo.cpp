#include "LocalGizmo.h"
#include "URenderer.h"

void LocalGizmo::Initialize(URenderer* renderer)
{
	Transform.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	Transform.SetScale(FVector(1.0f, 1.0f, 1.0f));
	Transform.SetRotationDeg(FVector(0.0f, 0.0f, 0.0f));
	
   /* gizmoCubeVerticesBuffer = nullptr;
    gizmoCubeIndicesBuffer = nullptr;

    renderer->CreateVertexBuffer(gizmoCubeVerticesBuffer,
        GizmoCubeVertices.data(),
        static_cast<unsigned int>(GizmoCubeVertices.size() * sizeof(FVertexSimple)));

    renderer->CreateIndexBuffer(gizmoCubeIndicesBuffer,
        GizmoCubeIndices.data(),
        static_cast<unsigned int>(GizmoCubeIndices.size() * sizeof(unsigned int)));*/
}

void LocalGizmo::Render(URenderer* renderer)
{
}

void LocalGizmo::Release()
{
}
