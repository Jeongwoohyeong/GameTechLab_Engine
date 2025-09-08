#pragma once
#include <d3d11.h>
#include "UUIManager.h"
#include "Math.h"
#include "CScene.h"

class UD3dDevice;
class UShader;
struct ID3D11RasterizerState;
class UCamera;

class ShapeData;
class WorldGizmo;
class LocalGizmo;

class URenderer
{
public:
	URenderer();
	~URenderer() {};

	bool Initialize(HWND hWnd);
	void Render();
	void Release();

	bool CreateVertexBuffer(ID3D11Buffer** verticesBuffer, const void* vertices, unsigned int byteWidth);
	bool CreateIndexBuffer(ID3D11Buffer** indicesBuffer, const void* indices, unsigned int byteWidth);
	
	void SetTopology(bool isLine);

	void UpdateConstant(const FMatrix& mvp);
	void UpdateConstant(const FMatrix& mvp, const FVector& vec);
	void RenderMesh(ID3D11Buffer* VertexBuffer, unsigned int NumVertices, ID3D11Buffer* IndexBuffer, unsigned int IndexCount, unsigned int Stride);

private:
	UD3dDevice* Device = nullptr;
	UShader* Shader = nullptr;

	ShapeData* Shape = nullptr;

	LocalGizmo* localCube = nullptr;
	WorldGizmo* worldGizmo = nullptr;
	
	ID3D11RasterizerState* RasterizerState = nullptr;	
	UUIManager UI = {};
};