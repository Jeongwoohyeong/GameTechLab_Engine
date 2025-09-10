#pragma once
#include "Math.h"
#include "Gizmo.h"

struct ID3D11Buffer;
struct FVertexSimple;
class CInputManager;

class ScaleGizmo : public Gizmo
{
public: 
	axis AxisInfo[3] =
	{
		{ROTATE_X, TRANSLATE_X},
		{ROTATE_Y, TRANSLATE_Y},
		{ROTATE_Z, TRANSLATE_Z}
	};

	void Initialize(FTransform* transform);
	virtual void CreateAABB() override;

	FTransform* GetGizmoTransform();
	void CalculateTranslationOffset();
	FTransform UpdateGizmoTransformFromParent(axis a);

	void OnLMouseClick();
	void OnLMouseUnClick();

	FMatrix WorldMatrie = FMatrix::Identity();
	FVector PreviousMousePos;
	bool bIsStartMoving = false;

	CInputManager* InputManager;
	FTransform GizmoTransform[3];
};
