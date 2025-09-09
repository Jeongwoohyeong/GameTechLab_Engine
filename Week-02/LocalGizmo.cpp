#include "LocalGizmo.h"
#include "Gizmo.h"
#include "URenderer.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"
#include "UCamera.h"

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
    
    if (!bIsLocalMode)
    {
		// 누적된 회전을 초기화
        Transform.ClearRotation();
    }
    Transform.AddRotationDeg(AxisRotation);

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
    CInputManager::GetInstance().RegisterMouseClickCallback([this](FDragMouseData ClickInfo) -> void {
        this->OnLMouseClick(ClickInfo);
        });
    CInputManager::GetInstance().RegisterMouseDragCallback([this](FDragMouseData DragInfo) -> void {
        this->OnLMouseDrag(DragInfo);
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

void LocalGizmo::OnLMouseClick(FDragMouseData firstClickInfo)
{
    // UE_LOG("%f %f %f, clicked", firstClick.X, firstClick.Y, firstClick.Z);
    float distance = std::abs(ParentTransform->GetLocation().Z - UCamera::GetInstance().Location.Z);
    previousMousePos = UCamera::GetInstance().DeprojectScreenPoint(
        firstClickInfo.mouseX,
        firstClickInfo.mouseY,
        firstClickInfo.W,
        firstClickInfo.H,
        distance, //1.0,  //ParentTransform->GetLocation().Z,
        true
    );
}

void LocalGizmo::OnLMouseDrag(FDragMouseData dragInfo)
{
    if (SelectedAxis == -1)
        return;
    float distance = std::abs(ParentTransform->GetLocation().Z - UCamera::GetInstance().Location.Z);
    // UE_LOG("%d // %f %f %f , draged", SelectedAxis, dragPos.X, dragPos.Y, dragPos.Z);
    currentMousePos = UCamera::GetInstance().DeprojectScreenPoint(
        dragInfo.mouseX,
        dragInfo.mouseY,
        dragInfo.W,
        dragInfo.H ,
        distance, //1.0, //ParentTransform->GetLocation().Z,
        true
    );
    FVector newDelta = currentMousePos - previousMousePos;
    previousMousePos = currentMousePos;
    Scale(newDelta);
}

void LocalGizmo::Scale(FVector newDelta) // scale은 local이나 world가 없다.
{
    // 로컬 이동
    FMatrix srt = ParentTransform->GetTransformMatrix();
    FVector selectedVector;
    switch (SelectedAxis)
    {
    case 0: // z축
        selectedVector = { srt.M[2][0], srt.M[2][1], srt.M[2][2] }; // 셋째 행
        break;
    case 1: // y축
        selectedVector = { srt.M[1][0], srt.M[1][1], srt.M[1][2] }; // 둘째 행
        break;
    case 2: // x축
        selectedVector = { srt.M[0][0], srt.M[0][1], srt.M[0][2] }; // 첫 행
        break;
    }
    selectedVector.Normalize();
    float offset = Dot(newDelta, selectedVector);

    FVector resultVector = offset * selectedVector;
    ParentTransform->AddScale(resultVector);
}

void LocalGizmo::RotateLocalOrWorld(FVector newDelta)
{

}
void LocalGizmo::TranslateLocalOrWorld(FVector newDelta)
{
    //// 월드 이동
    //FVector selectedVector;
    //switch (SelectedAxis)
    //{
    //case 0 : // z축
    //    selectedVector = { 0, 0, 1 };
    //    break;
    //case 1: // y축
    //    selectedVector = { 0, 1, 0 };
    //    break;
    //case 2: // x축
    //    selectedVector = { 1, 0, 0 };
    //    break;
    //}

    // 로컬 이동
    FMatrix srt = ParentTransform->GetTransformMatrix();
    FVector selectedVector;
    switch (SelectedAxis)
    {
    case 0: // z축
        selectedVector = { srt.M[2][0], srt.M[2][1], srt.M[2][2] }; // 셋째 행
        break;
    case 1: // y축
        selectedVector = { srt.M[1][0], srt.M[1][1], srt.M[1][2] }; // 둘째 행
        break;
    case 2: // x축
        selectedVector = { srt.M[0][0], srt.M[0][1], srt.M[0][2] }; // 첫 행
        break;
    }

    // UE_LOG("%f %f %f , draged", selectedVector.X, selectedVector.Y, selectedVector.Z);
    selectedVector.Normalize();
    float offset = Dot(newDelta, selectedVector);

    FVector resultVector = offset * selectedVector;
    ParentTransform->AddLocation(resultVector);
}

void LocalGizmo::OnLMouseRelease()
{
    // UE_LOG("release");
    SelectedAxis = -1;
    previousMousePos = { 0, 0, 0 };
    currentMousePos = { 0, 0, 0 };
}