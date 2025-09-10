#include <Windows.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "URenderer.h"
#include "UPrimitiveComponent.h"
#include "CCameraMovementController.h"
#include "UCamera.h"
#include "CScene.h"
#include "CInputManager.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}
	URenderer* renderer = (URenderer*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (message)
	{
	case WM_DESTROY:
		// 앱 종료 신호
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (renderer)
		{
			renderer->Resize(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// URenderer, CCameraMovementController, Scene, UCamera 초기화
bool GlobalInit(HWND* InHWnd, URenderer* R, CCameraMovementController* C, uint32 ClientW, uint32 ClientH)
{
	CInputManager::GetInstance().SetHWnd(InHWnd);
	C->Initialize(InHWnd);
	CScene::GetInstance().New();
	if(!R->Initialize(*InHWnd)) return false;

	SetLastError(0);
	LONG_PTR Prev = SetWindowLongPtr(*InHWnd, GWLP_USERDATA, (LONG_PTR)R);
	if (Prev == 0 && GetLastError() != 0) return false; // SetWindowLongPtr 실패

	if ((float)ClientH > 1e-6)
	{
		UCamera::GetInstance().Init(FVector(0.0f, 0.0f, -5.0f), FVector::Zero(), 60.0f, (float)ClientW, (float)ClientH);
	}

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";

	// 메시지 처리 함수 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 초기 윈도우 크기 설정 (카메라 초기 종횡비 초기화에 재사용)
	uint32 InitialWidth = 1680;
	uint32 InitialHeight = 1024;
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, InitialWidth, InitialHeight,
		nullptr, nullptr, hInstance, nullptr);

	URenderer renderer;
	CCameraMovementController* input = new CCameraMovementController();
	bool bInitSuccess = GlobalInit(&hWnd, &renderer, input, InitialWidth, InitialHeight);

	if (bInitSuccess)
	{
		bool bIsExit = false;

		// --- Main Loop ---
		while (bIsExit == false)
		{
			MSG msg;

			// 처리할 메시지가 더 이상 없을 때 까지 수행
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{

				// 키 입력 메시지 번역
				TranslateMessage(&msg);

				// 메시지를 적절한 윈도우 프로시저에 전달, 메시지가 WndProc에 전달
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
				{
					bIsExit = true;
					break;
				}
			}

			CInputManager::GetInstance().Update(); // InputManager 업데이트
			input->UpdateInputToCamera(); // CameraInputMove -> TODO: 나중에 InputManager에 의해 가공된 입력 처리용으로 수정

			// ImGui가 마우스를 잡고 있거나 마우스가 ImGui 위에 있을 때는 피킹하지 않음
			ImGuiIO& io = ImGui::GetIO();
			const bool OverImgui = io.WantCaptureMouse
				|| ImGui::IsAnyItemHovered()
				|| ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

			// --- Picking ---
			// TODO: Scene에 Update() 함수 만들어서 그 안에서 처리하기 (더 좋은 구조 있으면 그 쪽으로)
			if (!OverImgui && CInputManager::GetInstance().IsKeyPressed(VK_LBUTTON)) // 왼쪽 버튼 클릭 시
			{
				int32 ClientW = CInputManager::GetInstance().GetClientW();
				int32 ClientH = CInputManager::GetInstance().GetClientH();
				POINT MousePos = CInputManager::GetInstance().GetMouseClientPos();

				int32 SelectedAxis = -1;
				if(CScene::GetInstance().GetSelectedPrimitive())
				{
					FLocalGizmo* Giz = CScene::GetInstance().GetSelectedPrimitive()->GetGizmo();
					SelectedAxis = CScene::GetInstance().PickGizmoAtMouse(MousePos.x, MousePos.y, ClientW, ClientH);
					UE_LOG("Picked Gizmo Axis: %d", SelectedAxis);
					Giz->SelectGizmoAxis(SelectedAxis);
				}

				// 기즈모 축이 클릭된 게 아니면 프리미티브 피킹
				if (SelectedAxis == -1)
				{
					uint32 PickedUUID = -2;
					UPrimitiveComponent* PickedPrim = CScene::GetInstance().PickUObjectAtMouse(MousePos.x, MousePos.y, ClientW, ClientH, PickedUUID);
					if (PickedUUID != -2 && PickedPrim)
					{
						CScene::GetInstance().SetSelectedPrimitiveByUUID(PickedUUID);
					}
					else
					{
						UE_LOG("main.cpp: No primitive picked.");
					}
				}
			}

			renderer.Render();
		}
	}

	renderer.Release();

	return 0;
}