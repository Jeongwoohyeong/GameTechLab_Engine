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

inline constexpr FVector GAxisColors[3] = {
	{ 0.0f, 1.0f, 0.0f }, // Y - G
	{ 0.0f, 0.0f, 1.0f },  // Z - B
	{ 1.0f, 0.0f, 0.0f }, // X - R
};

struct FGizmoProperty {
	FVector HeadScale;
	FMesh* SelectedGizmo;
	int GizmoSwitch;
	FGizmoProperty() :SelectedGizmo(nullptr), GizmoSwitch(0), HeadScale(FVector(0.1f, 0.8f, 0.1f)) {};
};

// 각종 버퍼와 전체 렌더링 플로우를 관리하는 클래스입니다.
// 정적 멤버변수로 기본 도형 Mesh를 보유합니다.
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
	void RenderScene();
	bool RenderPrimitive(UPrimitiveComponent* Primitive);
	bool RenderLocalGizmo(UPrimitiveComponent* Primitive);
	void RenderUI();
	void Render(FMesh* mesh, ID3D11DeviceContext* DeviceContext, FMatrix* World);

	// Mesh 관련 메서드
	bool CreateVertexBuffer(FMesh* Mesh);
	bool CreateIndexBuffer(FMesh* Mesh);

	bool CreateAllMesh();
	bool CreateMesh(FMesh*& mesh, FVertexSimple* vertices, int verticeSize, uint32* indices = nullptr, int indiceSize = 0);

	void ReleaseAllMesh();
	void ReleaseMesh(FMesh* mesh);

public:
	static FMesh* CubeMesh;
	static FMesh* SphereMesh;
	static FMesh* TriangleMesh;
	static FMesh* ConeMesh;
	static FMesh* CylinderMesh;
	static FMesh* GizmoCubeMesh;

private:
	WorldGizmo* worldGizmo = nullptr;
	UD3dDevice* Device = nullptr;
	UShader* Shader = nullptr;
	ID3D11RasterizerState* RasterizerState = nullptr;
	UUIManager UI = {};
	FGizmoProperty LocalGizmoProperty;
};