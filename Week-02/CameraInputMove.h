#pragma once
#include <Windows.h>

class CameraInputMove
{
public:
	void Initialize(HWND* InHWnd);
	void UpdateInputToCamera()
	{
		UpdateMouseDelta();
		ApplyToCamera();
	}

	float ToRadian(float Degree);

	HWND* hWnd = nullptr;
	POINT CurMouseDelta = { 0, 0 };

private:
	void UpdateMouseDelta();
	void ApplyToCamera();

	POINT PrevMousePos = { 0, 0 };
};

