#pragma once
#include "FTransform.h" 
#include "IntersectionTest.h"

class URenderer;
class Gizmo
{
public:
	Gizmo() = default;
	~Gizmo() = default;

	virtual void Initialize(URenderer* renderer) {};
	virtual void Render(URenderer* renderer) {};
	virtual void CreateAABB() {};
	virtual void Release() {};
	virtual void SelectGizmoAxis(int32 Axis) { SelectedAxis = Axis; } // -1: none, 0: X, 1: Y, 2: Z

	FTransform* ParentTransform; // 부모 UPrimitiveComponent의 Transform 주소
	FAABB AABB;
	bool bIsAABBCreated = false;
	int32 SelectedAxis = -1; // -1: none, 0: X, 1: Y, 2: Z 
};