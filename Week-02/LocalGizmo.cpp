#include "LocalGizmo.h"
#include "Gizmo.h"
#include "URenderer.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"
#include "UCamera.h"
#include "CInputManager.h"

 void LocalGizmo::Initialize(URenderer* renderer, FTransform* transform)
{
    ParentTransform = transform;

    inputManager = &CInputManager::GetInstance();
    inputManager->RegisterMouseClickCallback([this]() { this->OnLMouseClick(); });
    inputManager->RegisterMouseReleaseCallback([this]() { this->OnLMouseUnclick();});

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

	CreateAABB();
}

FTransform LocalGizmo::UpdateGizmoTranformFromParent(axis a)
{
    FTransform temp = *ParentTransform;
    FVector rot = a.rotate;

    temp.SetScale(0.2f, 0.4f, 0.2f);

    FVector resizedLocation = ParentTransform->GetLocation() + (a.direction) * ParentTransform->GetScale()*0.5f;
    temp.SetLocation(resizedLocation);
    // temp.SetRotationDeg({ 0.0f, 0.0f,0.0f });
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
        if (startMoving)
        {
            UE_LOG("Moving!");
            CalculateTranslationOffSet();
        }
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

void LocalGizmo::CalculateTranslationOffSet()
{
    if (SelectedAxis == -1)
        return;

    FVector currentMousePos = CInputManager::GetInstance().MouseCurrentPosWorld;
    FVector mouseDelta = currentMousePos - previousMousePos;
    
    float offset = Dot(axisInfo[SelectedAxis].direction, mouseDelta);

    TranslatePrimitive(SelectedAxis, offset);
    previousMousePos = currentMousePos;
}

void LocalGizmo::TranslatePrimitive(int axis, float offSet)
{
	FVector primitiveTranslateOffset = axisInfo[axis].direction * offSet;
    ParentTransform->AddLocation(primitiveTranslateOffset);
}

void LocalGizmo::OnLMouseClick()
{
    previousMousePos = CInputManager::GetInstance().MousePressPosWorld;
    startMoving = true;
}
void LocalGizmo::OnLMouseUnclick()
{
    SelectedAxis = -1;
    startMoving = false;
}

void LocalGizmo::CreateAABB()
{
    FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // coneVerts 순회
    for (const auto& v : coneVerts)
    {
        if (v.x < Min.X) Min.X = v.x;
        if (v.y < Min.Y) Min.Y = v.y;
        if (v.z < Min.Z) Min.Z = v.z;

        if (v.x > Max.X) Max.X = v.x;
        if (v.y > Max.Y) Max.Y = v.y;
        if (v.z > Max.Z) Max.Z = v.z;
    }

    // cylinderVerts 순회
    for (const auto& v : cylinderVerts)
    {
        if (v.x < Min.X) Min.X = v.x;
        if (v.y < Min.Y) Min.Y = v.y;
        if (v.z < Min.Z) Min.Z = v.z;

        if (v.x > Max.X) Max.X = v.x;
        if (v.y > Max.Y) Max.Y = v.y;
        if (v.z > Max.Z) Max.Z = v.z;
    }

    // 결과 저장
    AABB.Min = Min;
    AABB.Max = Max;
    bIsAABBCreated = true;
}