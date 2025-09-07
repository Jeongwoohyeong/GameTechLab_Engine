#pragma once
#include "FVertexStruct.h"
#include "UPrimitiveComponent.h"
#include "UCamera.h"
#include <vector>

class URenderer;
class ID3D11Buffer;
class Gizmo
{
public:
	Gizmo() = default;
	~Gizmo() = default;

	virtual void Initialize(URenderer* renderer) = 0;
	virtual void Render(URenderer* renderer) = 0;
	virtual void Release() = 0;

	FTransform Transform;
};