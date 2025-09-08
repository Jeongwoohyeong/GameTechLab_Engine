#include "LocalGizmo.h"
#include "URenderer.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"

void LocalGizmo::Initialize(URenderer* renderer)
{
    Transform.SetLocation({ 0,0,0 });
    Transform.SetScale({ 0.1,0.1,0.1 });
    Transform.SetRotationDeg({ 0,0,0 });

    // C 배열 → span으로 자동 래핑
    coneVerts = std::span<FVertexSimple>(GCubeVertices);
    coneIdx = std::span<unsigned int>(GCubeIndices);
    cylinderVerts = std::span<FVertexSimple>(GCylinderVertices);
    cylinderIdx = std::span<unsigned int>(GCylinderIndices);

    renderer->CreateVertexBuffer(
        &gizmoConeVerticesBuffer,
        coneVerts.data(),
        static_cast<unsigned int>(coneVerts.size_bytes())
    );

    renderer->CreateIndexBuffer(
        &gizmoConeIndicesBuffer,
        coneIdx.data(),
        static_cast<unsigned int>(coneIdx.size_bytes())
    );

    renderer->CreateVertexBuffer(
        &gizmoCylinderVerticesBuffer,
        cylinderVerts.data(),
        static_cast<unsigned int>(cylinderVerts.size_bytes())
    );

    renderer->CreateIndexBuffer(
        &gizmoCylinderIndicesBuffer,
        cylinderIdx.data(),
        static_cast<unsigned int>(cylinderIdx.size_bytes())
    );
}

void LocalGizmo::Render(URenderer* renderer)
{
    renderer->SetTopology(false);
    renderer->UpdateConstant(UCamera::GetInstance().MakeMVP(Transform.GetTransformMatrix()));
    renderer->RenderMesh(
        gizmoConeVerticesBuffer, static_cast<unsigned int>(coneVerts.size()),
        gizmoConeIndicesBuffer, static_cast<unsigned int>(coneIdx.size()),
        sizeof(FVertexSimple)
    );
    //renderer->RenderMesh(
    //    gizmoCylinderVerticesBuffer, static_cast<unsigned int>(cylinderVerts.size()),
    //    gizmoCylinderIndicesBuffer, static_cast<unsigned int>(cylinderIdx.size()),
    //    sizeof(FVertexSimple)
    //);
}

void LocalGizmo::Release()
{
	if (gizmoConeVerticesBuffer)
	{
		gizmoConeVerticesBuffer->Release();
		gizmoConeVerticesBuffer = nullptr;
	}
	if (gizmoConeIndicesBuffer)
	{
		gizmoConeIndicesBuffer->Release();
		gizmoConeIndicesBuffer = nullptr;
	}
    if (gizmoCylinderVerticesBuffer)
    {
        gizmoCylinderVerticesBuffer->Release();
        gizmoCylinderVerticesBuffer = nullptr;
	}
    if (gizmoCylinderIndicesBuffer)
    {
        gizmoCylinderIndicesBuffer->Release();
        gizmoCylinderIndicesBuffer = nullptr;
	}
}
