#pragma once
#include "UUIManager.h"
#include "Math.h"
#include "CScene.h"
#include "FMesh.h"

class UD3dDevice;
class UShader;
struct ID3D11RasterizerState;
class UCamera;
class UPrimitiveComponent;

class URenderer
{
public:
	URenderer();
	~URenderer() {};

	bool Initialize(HWND hWnd);
	void Render();
	void Release();
	bool CreateRasterizerState();

private:
	bool CreateVertexBuffer(FMesh* Mesh);
	bool CreateIndexBuffer(FMesh* Mesh);
	bool RenderPrimitive(UPrimitiveComponent* Primitive);

	bool CreateAllMesh();
	bool CreateCubeMesh();
	bool CreateSphereMesh();
	void ReleaseAllMesh();

public:
	static FMesh* CubeMesh;
	static FMesh* SphereMesh;

private:
	UD3dDevice* Device = nullptr;
	UShader* Shader = nullptr;	
	ID3D11RasterizerState* RasterizerState = nullptr;	
	UUIManager UI = {};

	// TODO: WorldGizmo 인터페이스 호환되지 않는 문제 해결해야 함
	// WorldGizmo* worldGizmo = nullptr;
	
};