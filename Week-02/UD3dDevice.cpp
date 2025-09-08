#include "UD3dDevice.h"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

bool UD3dDevice::Initialize(HWND hWnd)
{
	if (!this->CreateDeviceAndSwapChain(hWnd))	 return false;
	if (!this->CreateFrameBuffer())				 return false;
	if (!this->CreateDepthStencilBuffer())		 return false;
	if (!this->CreateDepthStates())              return false;
	if (!this->CreateRasterizerState())			 return false;

	return true;	
}

void UD3dDevice::Release()
{
	if (DepthStateOpaque) { DepthStateOpaque->Release();  DepthStateOpaque = nullptr; }
	if (DepthStencilView) { DepthStencilView->Release();  DepthStencilView = nullptr; }
	if (DepthStencilBuffer) { DepthStencilBuffer->Release();DepthStencilBuffer = nullptr; }
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
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);

	DeviceContext->ClearRenderTargetView(FrameBufferRTV, Color);
	DeviceContext->ClearDepthStencilView(DepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->RSSetViewports(1, &Viewport);
	DeviceContext->OMSetDepthStencilState(DepthStateOpaque, 0);
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

	ComPtr<ID3D11Texture2D> backBuffer;
	result = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(result)) return false;

	D3D11_TEXTURE2D_DESC bb{};
	backBuffer->GetDesc(&bb);
	BackBufferWidth = (UINT)bb.Width;
	BackBufferHeight = (UINT)bb.Height;

	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = static_cast<float>(BackBufferWidth);
	Viewport.Height = static_cast<float>(BackBufferHeight);
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
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.DepthBias = 1;
	rasterizerDesc.SlopeScaledDepthBias = 1.0f;   // 기울기에 따라 추가 보정
	rasterizerDesc.DepthBiasClamp = 0.0f;
	ID3D11RasterizerState* RSGrid = nullptr;
	result = Device->CreateRasterizerState(&rasterizerDesc, &RSGrid);
	
	if (FAILED(result))
	{
		MessageBox(nullptr, L"rasterizerstate create fail,", L"error", MB_OK);
		return false;
	}
	return true;
}

bool UD3dDevice::CreateDepthStencilBuffer()
{
	if (DepthStencilView) { DepthStencilView->Release();   DepthStencilView = nullptr; }
	if (DepthStencilBuffer) { DepthStencilBuffer->Release(); DepthStencilBuffer = nullptr; }

	// 백버퍼와 동일 크기/샘플링으로 생성
	DXGI_SWAP_CHAIN_DESC scd{};
	SwapChain->GetDesc(&scd);

	D3D11_TEXTURE2D_DESC ds{};
	ds.Width = BackBufferWidth;
	ds.Height = BackBufferHeight;
	ds.MipLevels = 1;
	ds.ArraySize = 1;
	ds.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // D32_FLOAT도 가능
	ds.SampleDesc = scd.SampleDesc;
	ds.Usage = D3D11_USAGE_DEFAULT;
	ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	HRESULT hr = Device->CreateTexture2D(&ds, nullptr, &DepthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"depth tex create fail", L"error", MB_OK);
		return false;
	}

	hr = Device->CreateDepthStencilView(DepthStencilBuffer, nullptr, &DepthStencilView);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"DSV create fail", L"error", MB_OK);
		return false;
	}

	return true;
}

bool UD3dDevice::CreateDepthStates()
{
	if (DepthStateOpaque) { DepthStateOpaque->Release();  DepthStateOpaque = nullptr; }

	// 불투명용: 테스트 ON, 쓰기 ON
	D3D11_DEPTH_STENCIL_DESC dss{};
	dss.DepthEnable = TRUE;
	dss.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dss.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dss.StencilEnable = FALSE;

	HRESULT hr = Device->CreateDepthStencilState(&dss, &DepthStateOpaque);
	if (FAILED(hr)) return false;

	return true;
}