#pragma once
#include "USceneComponent.h"
#include "Types.h"
#include "Math.h"
#include "UPrimitiveTypes.h"
#include "FTransform.h"
#include "IntersectionTest.h"
#include "FMesh.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;
class LocalGizmo;

class UPrimitiveComponent :public USceneComponent
{
public:
	UPrimitiveComponent();
	virtual ~UPrimitiveComponent() {};

	void Release();

	inline FTransform* GetTransform() { return &Transform; }

	virtual EPrimitiveType GetPrimitiveType() = 0;
	virtual FMesh* GetMesh() = 0;

	virtual void CreateAABB();

protected:
	std::unique_ptr<LocalGizmo> Gizmo;
	FTransform Transform;
	FAABB AABB;
};