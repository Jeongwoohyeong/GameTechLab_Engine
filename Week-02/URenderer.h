#pragma once
#include "UUIManager.h"
#include "Math.h"

class UD3dDevice;
class UShader;
class UMesh;
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
	bool CreateVertexBuffer();
	bool CreateIndexBuffer();

private:
	UD3dDevice* Device = nullptr;
	UShader* Shader = nullptr;
	UPrimitiveComponent* Primitives;
	UMesh* Mesh= nullptr;
	ID3D11RasterizerState* RasterizerState = nullptr;	
	UUIManager UI = {};
};