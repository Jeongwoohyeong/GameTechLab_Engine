#pragma once
#include <d3d11.h>

class UD3dDevice
{
public:
	UD3dDevice() {};
	~UD3dDevice() {};

	bool Initialize(HWND hWnd);
	void Release();
	void BeginScene(float r = 0.05f, float g = 0.05f, float b = 0.05f, float a = 1.0f);
	void EndScene();
	void SetRSState(ID3D11RasterizerState*);

	ID3D11Device* GetDeivce() const { return Device; }
	ID3D11DeviceContext* GetDeviceContext() const { return DeviceContext; }		

private:
	bool CreateDeviceAndSwapChain(HWND hWnd);
	bool CreateFrameBuffer();
	

private:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	IDXGISwapChain* SwapChain = nullptr;
	ID3D11Texture2D* FrameBuffer = nullptr;
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;
	D3D11_VIEWPORT Viewport = {};
};