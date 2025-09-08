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
	bool CreateCubeBuffers();

private:
	bool CreateVertexBuffer(ID3D11Buffer**);
	bool CreateIndexBuffer(ID3D11Buffer**);

public:
	static ID3D11Buffer* CubeVertexBuffer;
	static ID3D11Buffer* CubeIndexBuffer;

private:
	UD3dDevice* Device = nullptr;
	UShader* Shader = nullptr;
	UPrimitiveComponent* Primitives;	
	ID3D11RasterizerState* RasterizerState = nullptr;	
	UUIManager UI = {};

	
};