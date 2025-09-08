#pragma once
#include "FVertexStruct.h"
#include "UPrimitiveComponent.h"
#include "UCamera.h"
#include <vector>
#include <d3d11.h> 

class URenderer;
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