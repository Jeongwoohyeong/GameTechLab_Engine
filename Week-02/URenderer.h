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
class WorldGizmo;

class URenderer
{
public:
	URenderer();
	~URenderer() {};

	bool Initialize(HWND hWnd);
	void Render();
	void Release();
	bool CreateRasterizerState();

	void Resize(UINT, UINT);

#pragma region Gizmo 사용, 추후 수정
	bool CreateVertexBuffer(ID3D11Buffer** verticesBuffer, const void* vertices, unsigned int byteWidth);
	bool CreateIndexBuffer(ID3D11Buffer** indicesBuffer, const void* indices, unsigned int byteWidth);

	void SetTopology(bool isLine);
	void UpdateConstant(const FMatrix& mvp);
	void UpdateConstant(const FMatrix& mvp, const FVector& vec);
	void RenderMesh(ID3D11Buffer* VertexBuffer, unsigned int NumVertices, ID3D11Buffer* IndexBuffer, unsigned int IndexCount, unsigned int Stride);
#pragma endregion

private:
	bool CreateVertexBuffer(FMesh* Mesh);
	bool CreateIndexBuffer(FMesh* Mesh);
	
	void RenderScene();
	bool RenderPrimitive(UPrimitiveComponent* Primitive);
	void RenderUI();

	bool CreateAllMesh();
	bool CreateCubeMesh();
	bool CreateSphereMesh();
	void ReleaseAllMesh();

public:
	static FMesh* CubeMesh;
	static FMesh* SphereMesh;

private:
	WorldGizmo* worldGizmo = nullptr;
	UD3dDevice* Device = nullptr;
	UShader* Shader = nullptr;
	ID3D11RasterizerState* RasterizerState = nullptr;
	UUIManager UI = {};

	// TODO: WorldGizmo 인터페이스 호환되지 않는 문제 해결해야 함
	// WorldGizmo* worldGizmo = nullptr;

};