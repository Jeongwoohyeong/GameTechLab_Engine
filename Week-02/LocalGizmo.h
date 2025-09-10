#pragma once
#include <span>
#include "Gizmo.h"
#include "Math.h"
#include "UUIManager.h"
#include "CInputManager.h"


struct ID3D11Buffer;
struct FVertexSimple;

class LocalGizmo : public Gizmo
{
public:
    axis axisInfo[3] =
    {
        {ROTATE_X, TRANSLATE_X}, // Y
        {ROTATE_Y, TRANSLATE_Y}, // Z
        {ROTATE_Z, TRANSLATE_Z} // X
    };

    ~LocalGizmo() { UnBind(); }; // 핸들 소멸자에서 자동 해제됨

    void Initialize(FTransform* transform);
	virtual void CreateAABB() override;
    void Bind();
    void UnBind();

    FTransform* GetGizmoTransform();
    FTransform UpdateGizmoTranformFromParent(axis a); // 기즈모 Transform Getter
    
    void CalculateTranslationOffSet();

    void OnLMouseClick(FDragMouseData firstClickInfo);
    void OnLMouseDrag(FDragMouseData dragInfo);
    void OnLMouseRelease();

    void Scale(FVector newDelta);
    void RotateLocalOrWorld(FVector newDelta);
    void TranslateLocalOrWorld(FVector newDelta);

    float CheckAndMarkScaleMinus(int& isScaleMinus, float addingScale, float parentScale);
    void SelectGizmo(int gizmoSwitch);

    FMatrix worldMatrix{
        1, 1 ,0,
        0, 1, 0,
        0, 0, 1
    };

    FVector previousMousePos;
    FVector currentMousePos;

    bool startMoving = false;
	bool bIsLocalMode = false; // 로컬 좌표계 모드 여부

    FTransform gizmoTransform[3];

    int GizmoSwitch = 0;

    int bScaleXMinus = 1;
    int bScaleYMinus = 1;
    int bScaleZMinus = 1;

    int OnClickIdx;
    int OnDragIdx;
    int OnReleaseIdx;
};