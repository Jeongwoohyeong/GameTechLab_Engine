#include "Time.h"
#include "App.h"
#include "ObjectFactory.h"
#include "MainMenuState.h" // 초기 상태 설정을 위해 포함
#include "SoundManager.h"
#include "Background.h"

LPCWSTR SpriteShaderFileName = L".\\SpriteShader.hlsl";
LPCWSTR SpriteAtlasShaderFileName = L".\\SpriteAtlasShader.hlsl";
string SpriteAtlasJsonPath = ".\\Resource\\sprite_sheet.json";
LPCWSTR SpriteBackgroundFilePath = L".\\Resource\\background.png";
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
	case WM_GETMINMAXINFO: {
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		RECT rect;
		GetClientRect(hWnd, &rect);

		int currentWidth = rect.right - rect.left;
		int currentHeight = rect.bottom - rect.top;

		double ratio = (double)currentWidth / currentHeight;

		if (ratio > 1) {
			// 현재 비율이 16:9보다 크면 (즉, 가로가 너무 길면), 높이를 기준으로 너비를 계산
			lpMMI->ptMaxTrackSize.x = (LONG)(currentHeight * 1);
			lpMMI->ptMaxTrackSize.y = currentHeight;
		}
		else {
			// 현재 비율이 16:9보다 작으면 (즉, 세로가 너무 길면), 너비를 기준으로 높이를 계산
			lpMMI->ptMaxTrackSize.x = currentWidth;
			lpMMI->ptMaxTrackSize.y = (LONG)(currentWidth * 1);
		}

		// 창의 최소 크기 설정 (옵션)
		lpMMI->ptMinTrackSize.x = 1024; // 최소 너비
		lpMMI->ptMinTrackSize.y = 1024; // 최소 높이 (16:9 비율 유지)
		break;
	}
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

	// 게임 시작 시 초기 상태를 MainMenuState로 설정
	ChangeState(new MainMenuState());
}
void UApp::MainLoop()
{
	Update();
	Render();
}

void UApp::ChangeState(IGameState* newState)
{
	// 다음 프레임에 상태가 변경되도록 예약합니다.
	if (newState)
	{
		nextState = newState;
	}
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

	if (FAILED(hResult))
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
	
	D3DUtil::CreateVSAndInputLayout(SpriteAtlasShaderFileName, &SpriteAtlasVS, &SpriteAtlasInputLayout);
	D3DUtil::CreatePS(SpriteAtlasShaderFileName, &SpriteAtlasPS);

	// Create Constant Buffer
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = sizeof(FMatrix4x4);
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Device->CreateBuffer(&constantbufferdesc, nullptr, &TransformCBuffer);

	//Create SamplerState
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	D3DUtil::CreateSamplerState(&SpriteSampleState, samplerDesc);

}
void UApp::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("Resource/NanumGothic-Bold.ttf", 24.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	// ImGui 백엔드 초기화
	ImGui_ImplWin32_Init((void*)HWnd);
	ImGui_ImplDX11_Init(Device, DeviceContext);
}
void UApp::Loading()
{
	QuadMesh = new UMesh(FMeshData::QuadMeshData);
	CircleMesh = new UMesh(FMeshData::CircleMeshData);

	SpriteSheet.Load(SpriteAtlasJsonPath);
	wstring atlasSpritePath = SpriteSheet.GetImagePath();
	LPCWSTR atlasSpritePathLPCWSTR = atlasSpritePath.c_str();

	D3DUtil::LoadTexture(atlasSpritePathLPCWSTR, &AtlasSRV);
	D3DUtil::LoadTexture(SpriteBackgroundFilePath, &BackgroundSRV);
	USoundManager::GetInstance()->Init();
}
void UApp::Start()
{
	UTime::GetInstance()->Init();
	USoundManager::GetInstance()->PlayBGM(SOUND_KEY_BGM);
}
void UApp::Update()
{
	UTime::GetInstance()->Update();
	UInput::GetInstance()->Update();

	float deltaTime = UTime::GetInstance()->GetDeltaTime();

	// 상태 전환 로직
	if (nextState != nullptr)
	{
		if (currentState != nullptr)
		{
			currentState->Exit();
			delete currentState;
		}
		currentState = nextState;
		currentState->Enter();
		nextState = nullptr;
	}

	// 현재 상태의 업데이트 로직 실행
	if (currentState)
	{
		currentState->Update(deltaTime);
	}
}
void UApp::RenderUI()
{
}
void UApp::Render()
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

	DeviceContext->RSSetViewports(1, &ViewportInfo);
	DeviceContext->RSSetState(RasterizerState);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformCBuffer);

	DeviceContext->PSSetSamplers(0, 1, &SpriteSampleState);
	DeviceContext->PSSetShaderResources(0, 1, &AtlasSRV);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 현재 상태의 렌더링 로직 실행
	if (currentState)
	{
		currentState->Render();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	SwapChain->Present(1, 0);
}

void UApp::Release()
{
	// 현재 상태 정리
	if (currentState != nullptr)
	{
		currentState->Exit();
		delete currentState;
		currentState = nullptr;
	}

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
