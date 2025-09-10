#include "UD3dDevice.h"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

bool UD3dDevice::Initialize(HWND hWnd)
{
	if (!this->CreateDeviceAndSwapChain(hWnd))	  return false;
	if (!this->CreateFrameBuffer())				  return false;
	if (!this->CreateDepthStencilBuffer())		  return false;
	if (!this->CreateDepthStates())               return false;
	if (!this->CreateRasterizerState())			  return false;
	if (!this->CreateOutlineDepthStencilState())  return false;
	if (!this->CreateGizmoDepthStencilState())	  return false;
	if (!this->CreateGizmoDepthStencilBuffer())	  return false;
	return true;	
}

void UD3dDevice::Release()
{
	if (GizmoDepthState) { GizmoDepthState->Release(); GizmoDepthState = nullptr; }
	if (GizmoDepthStencilView) { GizmoDepthStencilView->Release(); GizmoDepthStencilView = nullptr; }
	if (GizmoDepthStencilBuffer) { GizmoDepthStencilBuffer->Release(); GizmoDepthStencilBuffer = nullptr; }
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
    // 렌더 타겟/깊이 버퍼/뷰포트/블렌드 상태 설정 및 클리어
    float Color[] = { r,g,b, a };

    // 렌더 타겟과 깊이 스텐실 뷰 설정
    DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);

    // 렌더 타겟 클리어
    DeviceContext->ClearRenderTargetView(FrameBufferRTV, Color);
    // 깊이/스텐실 버퍼 클리어
    DeviceContext->ClearDepthStencilView(DepthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 뷰포트 설정
    DeviceContext->RSSetViewports(1, &Viewport);
    // 깊이 스텐실 상태 설정
    DeviceContext->OMSetDepthStencilState(DepthStateOpaque, 0);
    // 프리미티브 토폴로지 설정
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // 블렌드 상태 설정
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

void UD3dDevice::Resize(UINT width, UINT height)
{
	if (SwapChain == nullptr || DeviceContext == nullptr || Device == nullptr)
	{
		MessageBox(nullptr, L"swapchain nullptr", L"error", MB_OK);
		return;
	}
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	
	if (DepthStencilView)
	{
		DepthStencilView->Release();
		DepthStencilView = nullptr;
	}

	if (DepthStencilBuffer)
	{
		DepthStencilBuffer->Release();
		DepthStencilBuffer = nullptr;
	}

	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}

	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}	

	BackBufferWidth = width;
	BackBufferHeight = height;

	HRESULT result = SwapChain->ResizeBuffers(2, width, height,	DXGI_FORMAT_B8G8R8A8_UNORM, 0);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"resize fail", L"error", MB_OK);
		exit(-1);
	}

	if (!this->CreateFrameBuffer())
	{		
		return;
	}

	if (!this->CreateDepthStencilBuffer())
	{
		return;
	}

	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = static_cast<FLOAT>(BackBufferWidth);
	Viewport.Height = static_cast<FLOAT>(BackBufferHeight);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
}

bool UD3dDevice::CreateGizmoDepthStencilState()
{
	if (GizmoDepthState) { GizmoDepthState->Release();  GizmoDepthState = nullptr; }

	// 불투명용: 테스트 ON, 쓰기 ON
	D3D11_DEPTH_STENCIL_DESC dss{};
	dss.DepthEnable = FALSE;
	dss.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dss.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dss.StencilEnable = FALSE;

	HRESULT hr = Device->CreateDepthStencilState(&dss, &GizmoDepthState);
	if (FAILED(hr)) return false;

	return true;
}

bool UD3dDevice::CreateGizmoDepthStencilBuffer()
{
	if (GizmoDepthStencilView) { GizmoDepthStencilView->Release();   GizmoDepthStencilView = nullptr; }
	if (GizmoDepthStencilBuffer) { GizmoDepthStencilBuffer->Release(); GizmoDepthStencilBuffer = nullptr; }

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

	HRESULT hr = Device->CreateTexture2D(&ds, nullptr, &GizmoDepthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"Gizmo depth tex create fail", L"error", MB_OK);
		return false;
	}

	hr = Device->CreateDepthStencilView(GizmoDepthStencilBuffer, nullptr, &GizmoDepthStencilView);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"Gizmo DSV create fail", L"error", MB_OK);
		return false;
	}

	return true;
}

void UD3dDevice::BeginGizmo(ID3D11RasterizerState* gizmoRSState)
{
	// 기즈모셰이더 상태 설정
	DeviceContext->ClearDepthStencilView(DepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->OMSetDepthStencilState(GizmoDepthState, 0);

	DeviceContext->RSSetState(gizmoRSState);	
}

void UD3dDevice::EndGizmo(ID3D11RasterizerState* colorRSState)
{
	/// 컬러셰이더로 돌아가는 코드
	DeviceContext->OMSetDepthStencilState(DepthStateOpaque, 0);

	DeviceContext->RSSetState(colorRSState);
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

bool UD3dDevice::CreateOutlineDepthStencilState()
{
	ID3D11Device* dev = GetDeivce();

	// A) 스텐실 찍기(깊이 테스트만, 컬러 write OFF)
	D3D11_DEPTH_STENCIL_DESC dsA{};
	dsA.DepthEnable = TRUE;
	dsA.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsA.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsA.StencilEnable = TRUE;
	dsA.StencilReadMask = 0xFF;
	dsA.StencilWriteMask = 0xFF;
	dsA.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsA.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsA.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsA.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsA.BackFace = dsA.FrontFace;
	HRESULT hr = dev->CreateDepthStencilState(&dsA, &DS_StencilMark);
	if (FAILED(hr)) return false;

	D3D11_BLEND_DESC bdOff{};
	bdOff.RenderTarget[0].BlendEnable = FALSE;
	bdOff.RenderTarget[0].RenderTargetWriteMask = 0x00; // 컬러 미기록
	dev->CreateBlendState(&bdOff, &BS_ColorOff);

	// B) 외곽선 패스(깊이 끄고, 스텐실!=1만 통과)
	D3D11_DEPTH_STENCIL_DESC dsB{};
	dsB.DepthEnable = FALSE;
	dsB.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsB.StencilEnable = TRUE;
	dsB.StencilReadMask = 0xFF;
	dsB.StencilWriteMask = 0x00;
	dsB.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsB.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsB.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsB.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsB.BackFace = dsB.FrontFace;
	HRESULT hr2 = dev->CreateDepthStencilState(&dsB, &DS_Outline);
	if (FAILED(hr2)) return false;

	// 외곽선용: 프런트면 컬링(백페이스만 남겨 “실루엣”이 도드라짐)
	D3D11_RASTERIZER_DESC rs{};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_FRONT;
	rs.DepthClipEnable = TRUE;
	dev->CreateRasterizerState(&rs, &RS_CullFront); // TODO#3-1: 실패시 return false 처리

	// (선택) 반투명 합성
	D3D11_BLEND_DESC bd{};
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	dev->CreateBlendState(&bd, &BS_Alpha); // TODO#3-2: 실패시 return false 처리

	return true;
}