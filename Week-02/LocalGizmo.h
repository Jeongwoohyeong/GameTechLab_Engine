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

    void Initialize(FTransform* transform);
	virtual void CreateAABB() override;
    void Bind();

    FTransform* GetGizmoTransform();
    FTransform UpdateGizmoTranformFromParent(axis a); // 기즈모 Transform Getter
    
    void CalculateTranslationOffSet();

    void OnLMouseClick(FDragMouseData firstClickInfo);
    void OnLMouseDrag(FDragMouseData dragInfo);
    void OnLMouseRelease();

    void Scale(FVector newDelta);
    void RotateLocalOrWorld(FVector newDelta);
    void TranslateLocalOrWorld(FVector newDelta);

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
};