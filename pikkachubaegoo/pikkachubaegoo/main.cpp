#include "App.h"
#include "CustomVector.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	UApp app;
	app.Init(hInstance);
	bool bIsExit = false;
	while (bIsExit == false)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}
		}

		app.MainLoop();
	}
	return 0;
}