#pragma once
#include "UUIManager.h"
#include "Math.h"

class UD3dDevice;
class UShader;
class UMesh;
struct ID3D11RasterizerState;
class UCamera;

class ShapeData;
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
	void UpdateConstant(const FMatrix& mvp);

private:
	UD3dDevice* Device = nullptr;
	UMesh* Mesh= nullptr;
	UShader* Shader = nullptr;

	ShapeData* Shape = nullptr;
	WorldGizmo* worldGizmo = nullptr;
	
	ID3D11RasterizerState* RasterizerState = nullptr;	
	UUIManager UI = {};
};