#include "UD3dDevice.h"

bool UD3dDevice::Initialize(HWND hWnd)
{
	if (!this->CreateDeviceAndSwapChain(hWnd))
	{
		return false;
	}
	if (!this->CreateFrameBuffer())
	{
		return false;
	}
	if (!this->CreateRasterizerState())
	{
		return false;
	}

	return true;	
}

void UD3dDevice::Release()
{
	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}

	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}
	
	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}

	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}
}

void UD3dDevice::BeginScene(float r, float g, float b, float a)
{
	float Color[] = { r,g,b, a };
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);

	DeviceContext->ClearRenderTargetView(FrameBufferRTV, Color);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void UD3dDevice::EndScene()
{
	SwapChain->Present(1, 0);
}

void UD3dDevice::SetRSState(ID3D11RasterizerState* rasterizeState)
{
	DeviceContext->RSSetViewports(1, &Viewport);
	DeviceContext->RSSetState(rasterizeState);
}

bool UD3dDevice::CreateDeviceAndSwapChain(HWND hWnd)
{
	HRESULT result;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
	swapChainDesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
	swapChainDesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
	swapChainDesc.BufferCount = 2; // 더블 버퍼링
	swapChainDesc.OutputWindow = hWnd; // 렌더링할 창 핸들
	swapChainDesc.Windowed = TRUE; // 창 모드
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

	
	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
				featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
				&swapChainDesc, &SwapChain, &Device, nullptr, &DeviceContext);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"device create fail,", L"error", MB_OK);
		return false;
	}

	SwapChain->GetDesc(&swapChainDesc);

	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = (float)swapChainDesc.BufferDesc.Width;
	Viewport.Height = (float)swapChainDesc.BufferDesc.Height;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	return true;
}

bool UD3dDevice::CreateFrameBuffer()
{
	HRESULT result;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC frameBufferRTVdesc = {};
	frameBufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	frameBufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	result = Device->CreateRenderTargetView(FrameBuffer, &frameBufferRTVdesc, &FrameBufferRTV);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"RTV create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

bool UD3dDevice::CreateRasterizerState()
{
	HRESULT result;
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	result = Device->CreateRasterizerState(&rasterizerDesc, nullptr);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"rasterizerstate create fail,", L"error", MB_OK);
		return false;
	}
	return true;
}