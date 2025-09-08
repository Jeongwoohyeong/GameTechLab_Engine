#pragma once
#include "USceneComponent.h"
#include "Types.h"
#include "Math.h"
#include "UPrimitiveTypes.h"
#include "FTransform.h"
#include "IntersectionTest.h"
#include "FMesh.h"
#include "LocalGizmo.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;

class URenderer;

class UPrimitiveComponent :public USceneComponent
{
public:
	UPrimitiveComponent();
	virtual ~UPrimitiveComponent() = default;

	void Initialize(URenderer* renderer);
	void Render(URenderer* renderer);
	void Release();

	inline FTransform* GetTransform() { return &Transform; }

	virtual EPrimitiveType GetPrimitiveType() = 0;
	virtual FMesh* GetMesh() = 0;

	virtual void CreateAABB();

protected:
	LocalGizmo Gizmo;
	FAABB AABB;
};