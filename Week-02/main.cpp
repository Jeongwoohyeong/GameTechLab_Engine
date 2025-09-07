#include <Windows.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "URenderer.h"
#include "UPrimitiveComponent.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_DESTROY:
		// 앱 종료 신호
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";

	// 메시지 처리 함수 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 1024 x 1024 크기 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	URenderer renderer;
	UPrimitiveComponent* Cube = new UPrimitiveComponent();
	

	if (renderer.Initialize(hWnd))
	{
		bool bIsExit = false;
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

			renderer.Render();
		}
	}

	renderer.Release();

	return 0;
}