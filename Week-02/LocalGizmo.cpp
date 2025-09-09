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

    CreateAABB();

    Bind();
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
    FTransform Transform = *ParentTransform;
    FVector AxisRotation = a.rotate;

    Transform.SetScale(0.05f, 0.8f, 0.05f);

    FVector resizedLocation = ParentTransform->GetLocation();
    Transform.SetLocation(resizedLocation);
    
    if (bIsLocalMode)
    {
        // 로컬 좌표계 모드
        Transform.AddRotationDeg(AxisRotation);
    }
    else
    {
        // 월드 좌표계 모드
        Transform.SetRotationDeg(AxisRotation);
    }

    return Transform;
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

void LocalGizmo::Bind()
{
    CInputManager::GetInstance().RegisterMouseClickCallback([this](FVector worldPos) -> void {
        this->OnLMouseClick(worldPos); 
        });
    CInputManager::GetInstance().RegisterMouseDragCallback([this](FVector worldPos) -> void {
        this->OnLMouseDrag(worldPos);
        });
    CInputManager::GetInstance().RegisterMouseReleaseCallback([this]()->void { 
        this->OnLMouseRelease();
        });
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

void LocalGizmo::OnLMouseClick(FVector firstClick)
{
    UE_LOG("%f %f %f, clicked", firstClick.X, firstClick.Y, firstClick.Z);
    previousMousePos = firstClick;
}

void LocalGizmo::OnLMouseDrag(FVector dragPos)
{
    if (SelectedAxis == -1)
        return;
    UE_LOG("%d // %f %f %f , draged", SelectedAxis, dragPos.X, dragPos.Y, dragPos.Z);

    currentMousePos = dragPos;
    FVector newDelta = currentMousePos - previousMousePos;
    previousMousePos = currentMousePos;

    // 월드 이동
    FVector selectedVector;
    switch (SelectedAxis)
    {
    case 0 : // z축
        selectedVector = { 0, 0, 1 };
        break;
    case 1: // y축
        selectedVector = { 0, 1, 0 };
        break;
    case 2: // x축
        selectedVector = { 1, 0, 0 };
        break;
    }
    UE_LOG("%f %f %f , draged", selectedVector.X, selectedVector.Y, selectedVector.Z);
    selectedVector.Normalize();
    float offset = Dot(newDelta, selectedVector);

    FVector resultVector = offset * selectedVector;
    ParentTransform->AddLocation(resultVector);
}
void LocalGizmo::OnLMouseRelease()
{
    UE_LOG("release");
    SelectedAxis = -1;
    previousMousePos = { 0, 0, 0 };
    currentMousePos = { 0, 0, 0 };
}