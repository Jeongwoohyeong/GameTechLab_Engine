#pragma once
#include "FVertexStruct.h"
#include "UPrimitiveComponent.h"
#include <vector>


class Gizmo
{
public:
	Gizmo() = default;
	~Gizmo() = default;

	void Initialize(URenderer* renderer);
	void Render(URenderer* renderer);
	void Release();

	FTransform Transform;

private:
	std::vector<FVertexSimple>* vertices;
	std::vector<unsigned int>*  xzGridIndices;

	ID3D11Buffer* cubeVerticesBuffer = nullptr;
	ID3D11Buffer* cubeIndicesBuffer = nullptr;
};

