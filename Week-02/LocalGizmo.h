#pragma once
#include <span>
#include "Math.h"
#include "FTransform.h"
#include "UUIManager.h"
#include "CInputManager.h"
#include "IntersectionTest.h"

struct ID3D11Buffer;
struct FVertexSimple;

constexpr FVector ROTATE_X{ 90.0f, 0.0f, 0.0f }; // x축, 빨간색
constexpr FVector ROTATE_Y{ 0.0f, 0.0f, 0.0f }; // y축, 초록색
constexpr FVector ROTATE_Z{ 0.0f, 0.0f, -90.0f }; // z축, 파란색

constexpr FVector TRANSLATE_Z{ 1.0f, 0.0f, 0.0f }; // x축, 빨간색
constexpr FVector TRANSLATE_Y{ 0.0f, 1.0f, 0.0f }; // y축, 초록색
constexpr FVector TRANSLATE_X{ 0.0f, 0.0f, 1.0f }; // z축, 파란색

constexpr FVector RINGROTATE_Z{ 90.0f, 0.0f, 90.0f }; // x축, 빨간색
constexpr FVector RINGROTATE_Y{ 0.0f, 0.0f, 90.0f }; // y축, 초록색
constexpr FVector RINGROTATE_X{ 90.0f, 0.0f, 0.0f }; // z축, 파란색



struct axis
{
    FVector rotate;
    FVector direction;
    FVector ringRotate;
};

class LocalGizmo
{
public:
#pragma region 변수
    axis axisInfo[3] =
    {
        {ROTATE_X, TRANSLATE_X, RINGROTATE_X}, // X
        {ROTATE_Y, TRANSLATE_Y, RINGROTATE_Y}, // Y
        {ROTATE_Z, TRANSLATE_Z, RINGROTATE_Z} // Z
    };

    FTransform* ParentTransform; // 부모 UPrimitiveComponent의 Transform 주소
    FAABB AABB;
    bool bIsAABBCreated = false;

    FVector previousMousePos; // 기즈모 전용 드래그용 변수
    FVector currentMousePos;

    int GizmoSwitch = 0; // gizmo 종류 switch 0 : translate, 1: scale, 2: rotate
    int32 SelectedAxis = -1; // -1: none, 0: X, 1: Y, 2: Z 
    bool bIsLocalMode = false; // 로컬 좌표계 모드 여부

    FTransform gizmoTransform[3]; // 렌더용 transform

    int bScaleXMinus = 1; // scale 음수값 확인용 변수들
    int bScaleYMinus = 1;
    int bScaleZMinus = 1;

    int OnClickIdx; // input call back 해제용 인덱스
    int OnDragIdx;
    int OnReleaseIdx;
#pragma endregion

    ~LocalGizmo() { UnBind(); }; // 핸들 소멸자에서 자동 해제됨

    void Initialize(FTransform* transform);
    void CreateAABB();
    void Bind();
    void UnBind();

    void SelectGizmo(int gizmoSwitch) { GizmoSwitch = gizmoSwitch; } // 
    void SelectGizmoAxis(int32 Axis) { SelectedAxis = Axis; }

    FTransform* GetGizmoTransform();
    FTransform UpdateGizmoTranformFromParent(axis a); // 기즈모 Transform Getter

    void OnLMouseClick(FDragMouseData firstClickInfo);
    void OnLMouseDrag(FDragMouseData dragInfo);
    void OnLMouseRelease();

    void Scale(FVector newDelta);
    void RotateLocalOrWorld(FVector newDelta);
    void TranslateLocalOrWorld(FVector newDelta);

    float CheckAndMarkScaleMinus(int& isScaleMinus, float addingScale, float parentScale);
};