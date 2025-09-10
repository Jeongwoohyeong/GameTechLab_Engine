#pragma once
#include "USceneComponent.h"
#include "Types.h"
#include "Math.h"
#include "UPrimitiveTypes.h"
#include "FTransform.h"
#include "IntersectionTest.h"
#include "FMesh.h"
#include "LocalGizmo.h"
#include "UUIManager.h"
#include "RTTIMacros.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;

class URenderer;

class UPrimitiveComponent :public USceneComponent
{
	RTTI_DECLARE()
public:
	UPrimitiveComponent();
	virtual ~UPrimitiveComponent() = default;
	void Release();

	inline FTransform* GetTransform() { return &Transform; }
	inline FTransform* GetGizmoTransforms() { 
		if (LocationGizmo.ParentTransform == nullptr)
		{
			LocationGizmo.Initialize(&Transform);
		}
		return LocationGizmo.GetGizmoTransform();
	}

	virtual EPrimitiveType GetPrimitiveType() = 0;
	virtual FMesh* GetMesh() = 0;

	// RTTI 아님, 단순한 타입체크
	virtual void TypeCheck() = 0;

	virtual void CreateAABB();
	FAABB GetAABB();


	FLocalGizmo* GetGizmo() { return &LocationGizmo; }
	void SwitchGizmo(int gizmoSwitch);

protected:
	FTransform* GizmoTransform;	
	FLocalGizmo LocationGizmo;
	FAABB AABB;
	bool bIsAABBCreated = false;
	int GizmoSwitch = 0;
};