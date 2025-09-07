#include "LocalGizmo.h"
#include "URenderer.h"

void LocalGizmo::Initialize(URenderer* renderer)
{
	Transform.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	Transform.SetScale(FVector(1.0f, 1.0f, 1.0f));
	Transform.SetRotationDeg(FVector(0.0f, 0.0f, 0.0f));
	
	GizmoCubeVertices = {
		{ -0.5f, -0.5f, -0.5f, 1,0,0,1 }, // v0
		{ 0.5f, -0.5f, -0.5f, 0,1,0,1 }, // v1
		{ 0.5f, 0.5f, -0.5f, 0,0,1,1 }, // v2
		{ -0.5f, 0.5f, -0.5f, 1,1,0,1 }, // v3
		{ -0.5f, -0.5f, 0.5f, 1,0,1,1 }, // v4
		{ 0.5f, -0.5f, 0.5f, 0,1,1,1 }, // v5
		{ 0.5f, 0.5f, 0.5f, 1,1,1,1 }, // v6
		{ -0.5f, 0.5f, 0.5f, 0,0,0,1 }, // v7
	};

	GizmoCubeIndices = {
			// Front (+Z)
			4, 5, 6, 4, 6, 7,
			// Back (-Z)
			1, 0, 3, 1, 3, 2,
			// Left (-X)
			0, 4, 7, 0, 7, 3,
			// Right (+X)
			5, 1, 2, 5, 2, 6,
			// Top (+Y)
			3, 7, 6, 3, 6, 2,
			// Bottom (-Y)
			0, 1, 5, 0, 5, 4
	};

    gizmoCubeVerticesBuffer = nullptr;
    gizmoCubeIndicesBuffer = nullptr;

    renderer->CreateVertexBuffer(&gizmoCubeVerticesBuffer,
        GizmoCubeVertices.data(),
        static_cast<unsigned int>(GizmoCubeVertices.size() * sizeof(FVertexSimple)));

    renderer->CreateIndexBuffer(&gizmoCubeIndicesBuffer,
        GizmoCubeIndices.data(),
        static_cast<unsigned int>(GizmoCubeIndices.size() * sizeof(unsigned int)));
}

void LocalGizmo::Render(URenderer* renderer)
{
	renderer->SetTopology(false); // 선으로 렌더링
	renderer->UpdateConstant(UCamera::GetInstance().MakeMVP(Transform.GetTransformMatrix()));
	renderer->RenderMesh(gizmoCubeVerticesBuffer,
		static_cast<unsigned int>(GizmoCubeVertices.size()),
		gizmoCubeIndicesBuffer,
		static_cast<unsigned int>(GizmoCubeIndices.size()),
		sizeof(FVertexSimple));
}

void LocalGizmo::Release()
{
	if (gizmoCubeVerticesBuffer)
	{
		gizmoCubeVerticesBuffer->Release();
		gizmoCubeVerticesBuffer = nullptr;
	}
	if (gizmoCubeIndicesBuffer)
	{
		gizmoCubeIndicesBuffer->Release();
		gizmoCubeIndicesBuffer = nullptr;
	}
}
