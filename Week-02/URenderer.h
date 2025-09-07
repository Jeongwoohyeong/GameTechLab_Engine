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

private:

private:
	UD3dDevice* Device = nullptr;;
	UShader* Shader = nullptr;;
	UMesh* Mesh= nullptr;

	ShapeData* Shape = nullptr;
	WorldGizmo* worldGizmo = nullptr;
	
	ID3D11RasterizerState* RasterizerState = nullptr;	
	UUIManager UI = {};
};