#pragma once
#include <Windows.h>
#include <iostream>

#include "D3DUtil.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <unordered_map>

#include "Mesh.h"
#include "inputclass.h"
#include "IGameState.h"
using namespace std;

class UApp
{
public:
	static UApp* Ins;
	HWND HWnd;
private:

	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	IDXGISwapChain* SwapChain = nullptr;

	ID3D11Texture2D* FrameBuffer = nullptr;
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;
	ID3D11ShaderResourceView* FrameBufferSRV = nullptr;

	ID3D11RasterizerState* RasterizerState = nullptr;
	D3D11_VIEWPORT ViewportInfo;
	float ClearColor[4] = { 0, 1, 0, 1 };

	ID3D11InputLayout* SpriteInputLayout;
	ID3D11VertexShader* SpriteVS;
	ID3D11PixelShader* SpritePS;
	ID3D11SamplerState* SpriteSampleState;

	UMesh* QuadMesh;
	UMesh* CircleMesh;
	ID3D11Buffer* TransformCBuffer;

	ID3D11Texture2D* TestTexture;
	ID3D11ShaderResourceView* TestTextureSRV;
<<<<<<< Updated upstream
<<<<<<< Updated upstream

	// --- 상태 관리 변수 ---
	IGameState* currentState = nullptr;
	IGameState* nextState = nullptr;
=======
	

>>>>>>> Stashed changes
=======
	

>>>>>>> Stashed changes
public:
	void Init(HINSTANCE hInstance);
	void MainLoop();

	// 상태 변경을 요청하는 공용 함수
	void ChangeState(IGameState* newState);

	ID3D11Device* GetDevice() const
	{
		return Device;
	}
	ID3D11DeviceContext* GetContext() const
	{
		return DeviceContext;
	}
	ID3D11Buffer* GetTransformCBuffer() const
	{
		return TransformCBuffer;
	}
	UMesh* GetQuadMesh() const
	{
		return QuadMesh;
	}
	UMesh* GetCircleMesh() const
	{
		return CircleMesh;
	}
private:
	void InitWindow(HINSTANCE hInstance);
	void InitDirect();
	void InitImGui();

	void Loading();
	void Start();
	void Update();
	void RenderUI();
	void Render();

	void Release();

};