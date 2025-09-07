#pragma once
#include "USceneComponent.h"
#include "Types.h"
#include "Math.h"
#include "UPrimitiveTypes.h"

class UPrimitiveComponent :public USceneComponent
{
public:
	UPrimitiveComponent() {};
	virtual ~UPrimitiveComponent() {};

	virtual EPrimitiveType GetPrimitiveType() = 0;
protected:
	const void* Vertices = nullptr;
	const void* Indices = nullptr;
	uint32 VertexByteWidth = 0;
	uint32 IndexByteWidth = 0;
	uint32 VertexStide = 0;
	uint32 IndexCount = 0;
};