
#include "Time.h"
#include "App.h"
#include "ObjectFactory.h"

LPCWSTR SpriteShaderFileName = L"SpriteShader.hlsl";
UApp* UApp::Ins = nullptr;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void UApp::Init(HINSTANCE hInstance)
{
	Ins = this;
	InitWindow(hInstance);
	InitDirect();
	InitImGui();

	Loading();
	Start();
}
void UApp::MainLoop()
{
	Update();
	Render();
}


void UApp::InitWindow(HINSTANCE hInstance)
{
	WCHAR WindowClassName[] = L"PIKKPIKKA";
	WCHAR TitleName[] = L"PIKKAPIKKA";

	WNDCLASSW WndClass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClassName };

	RegisterClassW(&WndClass);

	DWORD WindowStyle = WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	int windowPosX = CW_USEDEFAULT;
	int windowPosY = CW_USEDEFAULT;
	int windowWidth = 1024;
	int windowHeight = 1024;

	HWnd = CreateWindowExW(0, WindowClassName, TitleName, WindowStyle, windowPosX, windowPosY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
}
void UApp::InitDirect()
{
	//Create Device, Context, Swapchain
	D3D_FEATURE_LEVEL featureLevels[]{ D3D_FEATURE_LEVEL_11_0 };
	DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
	swapchainDesc.BufferDesc.Width = 0;
	swapchainDesc.BufferDesc.Height = 0;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.OutputWindow = HWnd;
	swapchainDesc.Windowed = true;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hResult = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
		featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapchainDesc, &SwapChain, &Device, nullptr, &DeviceContext);

	if(FAILED(hResult))
	{
		cout << "CreateDeviceSwapChain Failed" << endl;
		return;
	}

	SwapChain->GetDesc(&swapchainDesc);

	float minDepth = 0.0f;
	float maxDepth = 1.0f;
	ViewportInfo = { 0.0f, 0.0f, (float)swapchainDesc.BufferDesc.Width, (float)swapchainDesc.BufferDesc.Height, minDepth, maxDepth };

	//Create FrameBuffer
	hResult = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);
	if (FAILED(hResult))
	{
		cout << "SwapChain GetBuffer Failed" << endl;
		return;
	}

	D3D11_RENDER_TARGET_VIEW_DESC frameBufferRTVDesc = {};
	frameBufferRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	frameBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hResult = Device->CreateRenderTargetView(FrameBuffer, &frameBufferRTVDesc, &FrameBufferRTV);
	if (FAILED(hResult))
	{
		cout << "CreateRenderTargetView Failed" << endl;
		return;
	}

	//Create RasterizerState
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	Device->CreateRasterizerState(&rasterizerDesc, &RasterizerState);

	D3DUtil::CreateVSAndInputLayout(SpriteShaderFileName, &SpriteVS, &SpriteInputLayout);
	D3DUtil::CreatePS(SpriteShaderFileName, &SpritePS);

	// Create Constant Buffer
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = sizeof(FMatrix4x4);
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Device->CreateBuffer(&constantbufferdesc, nullptr, &TransformCBuffer);

}
void UApp::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)HWnd);
	ImGui_ImplDX11_Init(Device, DeviceContext);

}
void UApp::Loading()
{
	QuadMesh = new UMesh(FMeshData::QuadMeshData);
	CircleMesh = new UMesh(FMeshData::CircleMeshData);
}
void UApp::Start()
{
	UTime::GetInstance()->Init();
	UObjectFactory::GetInstance()->CreatePlayer(FVector3(-0.5f)); // Player 1
	UObjectFactory::GetInstance()->CreatePlayer(FVector3(0.5f)); // Player 2
	UObjectFactory::GetInstance()->CreateBall(FVector3(0.0, 0.9f)); // Ball
	UObjectFactory::GetInstance()->CreateWall(FVector3(0.0f, -0.9f), FVector3(1.0f, 3.0f)); // Wall
}
void UApp::Update()
{
	UTime::GetInstance()->Update();
	UInput::GetInstance()->Update();
	UObjectFactory::GetInstance()->Update(UTime::GetInstance()->GetDeltaTime());
}
void UApp::RenderUI()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("UI");

	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
void UApp::Render()
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

	DeviceContext->RSSetViewports(1, &ViewportInfo);
	DeviceContext->RSSetState(RasterizerState);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->IASetInputLayout(SpriteInputLayout);
	DeviceContext->VSSetShader(SpriteVS, nullptr, 0);
	DeviceContext->PSSetShader(SpritePS, nullptr, 0);
	UApp::Ins->GetContext()->VSSetConstantBuffers(0, 1, &TransformCBuffer);

	UObjectFactory::GetInstance()->Render();

	RenderUI();
	SwapChain->Present(1, 0);
}

void UApp::Release()
{
	//Release 추가 필요
	QuadMesh->Release();

	TransformCBuffer->Release();
	SpritePS->Release();
	SpriteVS->Release();
	SpriteInputLayout->Release();
	//RenderContext->
	RasterizerState->Release();
	FrameBufferRTV->Release();
	FrameBufferSRV->Release();
	FrameBuffer->Release();
	SwapChain->Release();
	DeviceContext->Release();
	Device->Release();


	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

}
