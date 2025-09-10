#include "ScaleGizmo.h"
#include "URenderer.h"
#include "Cylinder.h"
#include "Cube.h"
#include "UCamera.h"
#include "CInputManager.h"

void ScaleGizmo::Initialize(FTransform* transform)
{
    Gizmo::ParentTransform = transform;

    InputManager = &CInputManager::GetInstance();
    InputManager->RegisterMouseClickCallback([this]() { this->OnLMouseClick(); });
    InputManager->RegisterMouseReleaseCallback([this]() { this->OnLMouseUnClick(); });

    CreateAABB();
}

void ScaleGizmo::CreateAABB()
{
    FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& v : GCubeVertices)
    {
        if (v.x < Min.X) Min.X = v.x;
        if (v.y < Min.Y) Min.Y = v.y;
        if (v.z < Min.Z) Min.Z = v.z;

        if (v.x > Max.X) Max.X = v.x;
        if (v.y > Max.Y) Max.Y = v.y;
        if (v.z > Max.Z) Max.Z = v.z;
    }

    for (const auto& v : GCylinderVertices)
    {
        if (v.x < Min.X) Min.X = v.x;
        if (v.y < Min.Y) Min.Y = v.y;
        if (v.z < Min.Z) Min.Z = v.z;

        if (v.x > Max.X) Max.X = v.x;
        if (v.y > Max.Y) Max.Y = v.y;
        if (v.z > Max.Z) Max.Z = v.z;
    }

    AABB.Min = Min;
    AABB.Max = Max;
    bIsAABBCreated = true;
}

FTransform* ScaleGizmo::GetGizmoTransform()
{
    const int arraySize = sizeof(AxisInfo) / sizeof(AxisInfo[0]);
    GizmoTransform[arraySize];
    for (int i = 0; i < arraySize; i++)
    {
        GizmoTransform[i] = UpdateGizmoTransformFromParent(AxisInfo[i]);
    }
    return GizmoTransform;
}

void ScaleGizmo::CalculateTranslationOffset()
{
    if (SelectedAxis == -1)
        return;

    FVector currentMousePos = CInputManager::GetInstance().MouseCurrentPosWorld;
    FVector mouseDelta = currentMousePos - PreviousMousePos;

    float offset = Dot(AxisInfo[SelectedAxis].direction, mouseDelta);

    FVector primitiveTranslateOffset = AxisInfo[SelectedAxis].direction * offset;
    ParentTransform->AddLocation(primitiveTranslateOffset);
    PreviousMousePos = currentMousePos;
}

FTransform ScaleGizmo::UpdateGizmoTransformFromParent(axis a)
{
    FTransform temp = *ParentTransform;
    FVector rot = a.rotate;

    temp.SetScale(0.05f, 0.8f, 0.05f);

    FVector resizedLocation = ParentTransform->GetLocation();
    temp.SetLocation(resizedLocation);
    temp.AddRotationDeg(rot);

	return temp;
}

void ScaleGizmo::OnLMouseClick()
{
    PreviousMousePos = CInputManager::GetInstance().MousePressPosWorld;
    bIsStartMoving = true;
}

void ScaleGizmo::OnLMouseUnClick()
{
    SelectedAxis = -1;
    bIsStartMoving = false;
}