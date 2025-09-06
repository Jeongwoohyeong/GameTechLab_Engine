#pragma once
#include "UUIManager.h"
#include "Math.h"

class UD3dDevice;
class UShader;
class UMesh;
class ID3D11RasterizerState;

class URenderer
{
public:
	URenderer();
	~URenderer() {};

	bool Initialize(HWND hWnd);
	void Render();
	void Release();
	bool CreateRasterizerState();
	void MVP();
private:

private:
	UD3dDevice* Device = nullptr;;
	UShader* Shader = nullptr;;
	UMesh* Mesh= nullptr;
	ID3D11RasterizerState* RasterizerState = nullptr;
	UUIManager UI = {};
	FMatrix MVPMatrix = FMatrix::Identity();
};