#include "LocalGizmo.h"
#include "Gizmo.h"
#include "URenderer.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"
#include "UCamera.h"
#include "CInputManager.h"

 void LocalGizmo::Initialize(FTransform* transform)
{
    ParentTransform = transform;

    inputManager = &CInputManager::GetInstance();
    inputManager->RegisterMouseClickCallback([this]() { this->OnLMouseClick(); });
    inputManager->RegisterMouseReleaseCallback([this]() { this->OnLMouseUnclick();});

	CreateAABB();
}

FTransform* LocalGizmo::GetGizmoTransform()
{
    const int arraySize = sizeof(axisInfo) / sizeof(axisInfo[0]);
    gizmoTransform[arraySize];
    for (int i = 0; i < arraySize; i++)
    {
        gizmoTransform[i] = UpdateGizmoTranformFromParent(axisInfo[i]);
    }
    return gizmoTransform;
}
FTransform LocalGizmo::UpdateGizmoTranformFromParent(axis a)
{
    FTransform temp = *ParentTransform;
    FVector rot = a.rotate;

    temp.SetScale(0.2f, 0.4f, 0.2f);

    FVector resizedLocation = ParentTransform->GetLocation() + (a.direction) * ParentTransform->GetScale() * 0.5f;
    temp.SetLocation(resizedLocation);
    // temp.SetRotationDeg({ 0.0f, 0.0f,0.0f });
	temp.AddRotationDeg(rot);

    return temp;
}
void LocalGizmo::CalculateTranslationOffSet()
{
    if (SelectedAxis == -1)
        return;

    FVector currentMousePos = CInputManager::GetInstance().MouseCurrentPosWorld;
    FVector mouseDelta = currentMousePos - previousMousePos;
    
    float offset = Dot(axisInfo[SelectedAxis].direction, mouseDelta);

    FVector primitiveTranslateOffset = axisInfo[SelectedAxis].direction * offset;
    ParentTransform->AddLocation(primitiveTranslateOffset);
    previousMousePos = currentMousePos;
}

void LocalGizmo::CreateAABB()
{
    FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // coneVerts 순회
    for (const auto& v : GConeVertices)
    {
        if (v.x < Min.X) Min.X = v.x;
        if (v.y < Min.Y) Min.Y = v.y;
        if (v.z < Min.Z) Min.Z = v.z;

        if (v.x > Max.X) Max.X = v.x;
        if (v.y > Max.Y) Max.Y = v.y;
        if (v.z > Max.Z) Max.Z = v.z;
    }

    // cylinderVerts 순회
    for (const auto& v : GCylinderVertices)
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