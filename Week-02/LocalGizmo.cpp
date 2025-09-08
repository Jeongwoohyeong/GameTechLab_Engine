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
    Transform.SetRotationDeg({ -90,0,0 });
    Transform.SetRotationDeg({ 0, 0,90 });

    // C 배열 → span으로 자동 래핑
    coneVerts = std::span<FVertexSimple>(GConeVertices);
    coneIdx = std::span<unsigned int>(GConeIndices);
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

FTransform LocalGizmo::UpdateGizmoTranformFromParent(axis a)
{
    FTransform temp = Transform;
    FVector rot = a.rotate;

    temp.SetScale(0.1f, 0.2f, 0.1f);

    FVector resizedLocation = Transform.GetLocation() + (a.translate) * Transform.GetScale();
    temp.SetLocation(resizedLocation);

    temp.AddRotationDegX(rot.X);
    temp.AddRotationDegY(rot.Y);
    temp.AddRotationDegZ(rot.Z);

   return temp;
}
void LocalGizmo::Render(URenderer* renderer){
    renderer->SetTopology(false);

    FTransform temp;
   
    for (auto& a : axisInfo)
    {
		temp = UpdateGizmoTranformFromParent(a); // x, y, z축 회전 적용
        
        renderer->UpdateConstant(UCamera::GetInstance().MakeMVP(temp.GetTransformMatrix()), a.color);
        renderer->RenderMesh(
            gizmoConeVerticesBuffer, static_cast<unsigned int>(coneVerts.size()),
            gizmoConeIndicesBuffer, static_cast<unsigned int>(coneIdx.size()),
            sizeof(FVertexSimple)
        );
        renderer->RenderMesh(
            gizmoCylinderVerticesBuffer, static_cast<unsigned int>(cylinderVerts.size()),
            gizmoCylinderIndicesBuffer, static_cast<unsigned int>(cylinderIdx.size()),
            sizeof(FVertexSimple)
        );
    }
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
