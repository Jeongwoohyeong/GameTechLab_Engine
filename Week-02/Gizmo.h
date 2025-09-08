#pragma once
#include "FTransform.h" 

class URenderer;
class Gizmo
{
public:
	Gizmo() = default;
	~Gizmo() = default;

	virtual void Initialize(URenderer* renderer) {};
	virtual void Render(URenderer* renderer) {};
	virtual void Release() {};

	FTransform* Transform;
};