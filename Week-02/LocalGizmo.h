#pragma once
#include "Gizmo.h"

class LocalGizmo : public Gizmo
{
public:
	LocalGizmo() = default;
	~LocalGizmo() = default;

	void Initialize(URenderer* renderer);
	void Render(URenderer* renderer);
	void Release();

public:
	std::vector<FVertexSimple> GizmoCubeVertices;
	std::vector<unsigned int>  GizmoCubeIndices;

	ID3D11Buffer* gizmoCubeVerticesBuffer = nullptr;
	ID3D11Buffer* gizmoCubeIndicesBuffer = nullptr;
};

