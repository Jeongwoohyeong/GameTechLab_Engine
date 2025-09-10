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
	void Resize(UINT, UINT);

	ID3D11Device* GetDevice() const { return Device; }
	ID3D11DeviceContext* GetDeviceContext() const { return DeviceContext; }		

	// --- URenderer에서 상태 객체 이용해 외곽선 만들기 때문에 Getter 제공 ---
	ID3D11DepthStencilState* GetDepthStateOpaque() const { return DepthStateOpaque; }
	ID3D11DepthStencilState* GetDS_StencilMark()  const { return DS_StencilMark; }
	ID3D11DepthStencilState* GetDS_Outline()      const { return DS_Outline; }
	ID3D11BlendState* GetBS_ColorOff()     const { return BS_ColorOff; }
	ID3D11BlendState* GetBS_Alpha()        const { return BS_Alpha; }
	ID3D11RasterizerState* GetRS_CullFront()    const { return RS_CullFront; }

	// 기즈모 셰이더
	ID3D11DepthStencilState* GetDS_Gizmo() const { return GizmoDepthState; }
	void BeginGizmo(ID3D11RasterizerState* gizmoRSState);
	void EndGizmo(ID3D11RasterizerState* colorRSState);

private:
	bool CreateDeviceAndSwapChain(HWND hWnd);
	bool CreateFrameBuffer();
	bool CreateRasterizerState();

	// depth stencil buffer & view & state
	bool CreateDepthStencilBuffer();
	bool CreateDepthStates();
	bool CreateOutlineDepthStencilState();

	// 기즈모 셰이더
	bool CreateGizmoDepthStencilState();
	bool CreateGizmoDepthStencilBuffer();

public:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;

private:
	IDXGISwapChain* SwapChain = nullptr;
	ID3D11Texture2D* FrameBuffer = nullptr;
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;
	D3D11_VIEWPORT Viewport = {};

	ID3D11Texture2D* DepthStencilBuffer = nullptr;
	ID3D11DepthStencilView* DepthStencilView = nullptr;
	ID3D11DepthStencilState* DepthStateOpaque = nullptr;     // 깊이 쓰기 ON

	UINT BackBufferWidth = 0;
	UINT BackBufferHeight = 0;

	//--- 하이라이팅 외곽선 생성용 ---
	ID3D11DepthStencilState* DS_StencilMark = nullptr;
	ID3D11DepthStencilState* DS_Outline = nullptr;
	ID3D11BlendState* BS_ColorOff = nullptr;
	ID3D11BlendState* BS_Alpha = nullptr;
	ID3D11RasterizerState* RS_CullFront = nullptr;

	// 기즈모 셰이더
	ID3D11Texture2D* GizmoDepthStencilBuffer = nullptr;
	ID3D11DepthStencilView* GizmoDepthStencilView = nullptr;
	ID3D11DepthStencilState* GizmoDepthState = nullptr;
};