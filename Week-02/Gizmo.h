#pragma once
#include "FTransform.h" 
#include "IntersectionTest.h"

constexpr FVector ROTATE_Y{ 0.0f, 0.0f, 0.0f }; // y축, 초록색
constexpr FVector ROTATE_Z{ 0.0f, 0.0f, -90.0f }; // z축, 파란색
constexpr FVector ROTATE_X{ 90.0f, 0.0f, 0.0f }; // x축, 빨간색

constexpr FVector TRANSLATE_Y{ 0.0f, 1.0f, 0.0f }; // y축, 초록색
constexpr FVector TRANSLATE_X{ 0.0f, 0.0f, 1.0f }; // z축, 파란색
constexpr FVector TRANSLATE_Z{ 1.0f, 0.0f, 0.0f }; // x축, 빨간색

struct axis
{
	FVector rotate;
	FVector direction;
};

class URenderer;
class Gizmo
{
public:
	Gizmo() = default;
	~Gizmo() = default;

	virtual void Initialize(URenderer* renderer) {};
	virtual void CreateAABB() {};
	virtual void Release() {};
	virtual void SelectGizmoAxis(int32 Axis) { SelectedAxis = Axis; } // -1: none, 0: X, 1: Y, 2: Z

	FTransform* ParentTransform; // 부모 UPrimitiveComponent의 Transform 주소
	FAABB AABB;
	bool bIsAABBCreated = false;
	int32 SelectedAxis = -1; // -1: none, 0: X, 1: Y, 2: Z 
};