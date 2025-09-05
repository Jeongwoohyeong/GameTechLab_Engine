#pragma once
#include <d3d11.h>

class USwapChain
{
public:
	USwapChain() {};
	~USwapChain() {};

	void Initialize();

private:
	void CreateSwapChain(HWND hWnd);


private:
	IDXGISwapChain* SwapChain = nullptr;
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;
	D3D11_VIEWPORT ViewPort = {};
};