#include "LocalGizmo.h"
#include "URenderer.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"
#include "UCamera.h"

 void FLocalGizmo::Initialize(FTransform* transform)
{
    ParentTransform = transform;

    CreateAABB();

    Bind();
}

FTransform* FLocalGizmo::GetGizmoTransform()
{
    const int arraySize = sizeof(axisInfo) / sizeof(axisInfo[0]);
    // gizmoTransform[arraySize];
    for (int i = 0; i < arraySize; i++)
    {
        gizmoTransform[i] = UpdateGizmoTranformFromParent(axisInfo[i]);
    }
    return gizmoTransform;
}

FTransform FLocalGizmo::UpdateGizmoTranformFromParent(axis a)
{
    FTransform Transform = *ParentTransform;
    FVector AxisRotation = a.rotate;
    if (GizmoSwitch == 2)
    {
        AxisRotation = a.ringRotate;
    }

    Transform.SetScale(0.05f, 0.8f, 0.05f);

    FVector resizedLocation = ParentTransform->GetLocation();
    Transform.SetLocation(resizedLocation);
    
    if (bIsLocalMode)
    {
        Transform.AddRotationDeg(AxisRotation, true);
    }
    else
    {
        // 누적된 회전을 초기화
        Transform.ClearRotation();
		Transform.AddRotationDeg(AxisRotation, false);
    }

    return Transform;
}

void FLocalGizmo::CreateAABB()
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

#pragma region Input Callback Handler
void FLocalGizmo::Bind()
{
    OnClickIdx = CInputManager::GetInstance().RegisterMouseClickCallback([this](FDragMouseData ClickInfo) -> void {
        this->OnLMouseClick(ClickInfo);
        });
    OnDragIdx = CInputManager::GetInstance().RegisterMouseDragCallback([this](FDragMouseData DragInfo) -> void {
        this->OnLMouseDrag(DragInfo);
        });
    OnReleaseIdx = CInputManager::GetInstance().RegisterMouseReleaseCallback([this]()->void {
        this->OnLMouseRelease();
        });
}

void FLocalGizmo::UnBind()
{
    CInputManager::GetInstance().RemoveMouseClickCallback(OnClickIdx);
    CInputManager::GetInstance().RemoveMouseDragCallback(OnDragIdx);
    CInputManager::GetInstance().RemoveMouseReleaseCallback(OnReleaseIdx);
}

#pragma endregion

#pragma region Receiving MouseInput

void FLocalGizmo::OnLMouseClick(FDragMouseData firstClickInfo)
{
    // UE_LOG("%f %f %f, clicked", firstClick.X, firstClick.Y, firstClick.Z);

    if (ParentTransform == nullptr)
    {
        UE_LOG("ParentNull");
        return;
    }

    const FVector A = ParentTransform->GetLocation();
    const FVector B = UCamera::GetInstance().Location;

    FVector CameraLocation = UCamera::GetInstance().Location;
    FVector CameraRocation = UCamera::GetInstance().Rotation;

    FVector ViewGizmoPos = MultiplyVecMat(ParentTransform->GetLocation(), FMatrix::MakeView(CameraLocation, CameraRocation));
    float distance = ViewGizmoPos.Z;

    previousMousePos = UCamera::GetInstance().DeprojectScreenPoint(
        firstClickInfo.mouseX,
        firstClickInfo.mouseY,
        firstClickInfo.W,
        firstClickInfo.H,
        distance, //1.0,  //ParentTransform->GetLocation().Z,
        false
    );
}

void FLocalGizmo::OnLMouseDrag(FDragMouseData dragInfo)
{
    if (SelectedAxis == -1)
        return;

    FVector CameraLocation = UCamera::GetInstance().Location;
    FVector CameraRotation = UCamera::GetInstance().Rotation;
    // UE_LOG("Location %f %f %f , draged", CameraLocation.X, CameraLocation.Y, CameraLocation.Z);
    // UE_LOG("Rocation %f %f %f , draged", CameraRotation.X, CameraRotation.Y, CameraRotation.Z);

    FVector ViewGizmoPos = MultiplyVecMat(ParentTransform->GetLocation(), FMatrix::MakeView(CameraLocation, CameraRotation));
    float distance = ViewGizmoPos.Z;

    currentMousePos = UCamera::GetInstance().DeprojectScreenPoint(
        dragInfo.mouseX,
        dragInfo.mouseY,
        dragInfo.W,
        dragInfo.H,
        distance, //1.0, //ParentTransform->GetLocation().Z,
        false
    );
    FVector newDelta = currentMousePos - previousMousePos; // view space delta
    previousMousePos = currentMousePos;
    // UE_LOG("%d // %f // %f %f %f, draged", SelectedAxis, distance, newDelta.X, newDelta.Y, newDelta.Z);
    FVector resultInWorld = MultiplyVecMat(newDelta, FMatrix::MakeRotation(CameraRotation));

    // 테스트 코드
    switch (GizmoSwitch)
    {
    case 0:
        TranslateLocalOrWorld(resultInWorld);
        break;
    case 1:
        Scale(resultInWorld);
        break;
    default:
        break;
    }
}

void FLocalGizmo::OnLMouseRelease()
{
    // UE_LOG("release");
    SelectedAxis = -1;

    bScaleXMinus = 1;
    bScaleYMinus = 1;
    bScaleZMinus = 1;

    previousMousePos = { 0, 0, 0 };
    currentMousePos = { 0, 0, 0 };
}

#pragma endregion

#pragma region Manipulating Primitives
void FLocalGizmo::Scale(FVector newDelta) // scale은 local이나 world가 없다.
{
    FVector selectedVector;

    //// 1단계 : 로컬 축에 대해 내적, 변화량 구함
    FMatrix srt = ParentTransform->GetTransformMatrix();
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

    // 2단계 : 이후 해당 로컬축의 x, y, z에 올바르게 곱해서 반영
    switch (SelectedAxis)
    {
    case 0: // z축
        selectedVector = { 0, 0, 1 };
        break;
    case 1: // y축
        selectedVector = { 0, 1, 0 };
        break;
    case 2: // x축
        selectedVector = { 1, 0, 0 };
        break;
    }
    FVector addingScale = offset * selectedVector;
    FVector parentScale = ParentTransform->GetScale();

    FVector resultScale;

    resultScale.X = CheckAndMarkScaleMinus(bScaleXMinus, addingScale.X, parentScale.X);
    resultScale.Y = CheckAndMarkScaleMinus(bScaleYMinus, addingScale.Y, parentScale.Y);
    resultScale.Z = CheckAndMarkScaleMinus(bScaleZMinus, addingScale.Z, parentScale.Z);

    ParentTransform->SetScale(resultScale);
}

float FLocalGizmo::CheckAndMarkScaleMinus(int& isScaleMinus, float addingScale, float parentScale)
{
    float result;
    result = parentScale + isScaleMinus * addingScale;
    if (result < 0)
    {
        result *= -1;
        isScaleMinus *= -1;
    }

    return result;
}

void FLocalGizmo::RotateLocalOrWorld(FVector newDelta)
{
    FMatrix cameraRotation = FMatrix::MakeRotation(UCamera::GetInstance().Rotation);
    FVector cameraForwardVec = { cameraRotation[2][0], cameraRotation[2][1], cameraRotation[2][2] };

    FVector selectedVector;
    FMatrix selectedRotation = FMatrix::MakeRotation(ParentTransform->GetRotationRadians());
    if (bIsLocalMode)
    {
        switch (SelectedAxis)
        {
        case 0: // z축
            selectedVector = { selectedRotation.M[2][0], selectedRotation.M[2][1], selectedRotation.M[2][2] }; // 셋째 행
            break;
        case 1: // y축
            selectedVector = { selectedRotation.M[1][0], selectedRotation.M[1][1], selectedRotation.M[1][2] }; // 둘째 행
            break;
        case 2: // x축
            selectedVector = { selectedRotation.M[0][0], selectedRotation.M[0][1], selectedRotation.M[0][2] }; // 첫 행
            break;
        }
        selectedVector.Normalize();
    }
    else
    {
        switch (SelectedAxis)
        {
        case 0: // z축
            selectedVector = { 0, 0, 1 };
            break;
        case 1: // y축
            selectedVector = { 0, 1, 0 };
            break;
        case 2: // x축
            selectedVector = { 1, 0, 0 };
            break;
        }
    }

    FVector direction = Cross(cameraForwardVec, selectedVector);
    direction.Normalize();

    float offset = Dot(newDelta, direction);
    FVector rotatedDir = offset * direction;

    ParentTransform->AddRotationDeg(rotatedDir, bIsLocalMode);
}

void FLocalGizmo::TranslateLocalOrWorld(FVector newDelta)
{
    FVector selectedVector;
    if (bIsLocalMode)
    {
        //로컬 이동
        FMatrix srt = ParentTransform->GetTransformMatrix();
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

    }
    else
    {
        //// 월드 이동
        switch (SelectedAxis)
        {
        case 0: // z축
            selectedVector = { 0, 0, 1 };
            break;
        case 1: // y축
            selectedVector = { 0, 1, 0 };
            break;
        case 2: // x축
            selectedVector = { 1, 0, 0 };
            break;
        }
    }

    // UE_LOG("%f %f %f , draged", selectedVector.X, selectedVector.Y, selectedVector.Z);
    selectedVector.Normalize();
    float offset = Dot(newDelta, selectedVector);

    FVector resultVector = offset * selectedVector;
    ParentTransform->AddLocation(resultVector);
}
#pragma endregion
