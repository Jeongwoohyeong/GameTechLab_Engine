#pragma once
#include <Windows.h>
#include "UCamera.h" 

class CameraInputMove
{
public:
	CameraInputMove();
	~CameraInputMove() {};

	void Initialize(HWND* InHWnd);
	void UpdateInputToCamera()
	{
		UpdateKeyboardInput();
		UpdateMouseDelta();
		ApplyToCamera();
	}

	HWND* hWnd = nullptr;
	POINT CurMouseDelta = { 0, 0 };

private:
	void UpdateKeyboardInput();
	void UpdateMouseDelta();
	void ApplyToCamera();

	float ToRadian(float Degree);
	FVector Multiply(const FVector& vector, const FMatrix& matrix) const;

	UCamera& Camera;
	FVector KeyboardMoveDelta = { 0.0f, 0.0f, 0.0f };
	POINT PrevMousePos = { 0, 0 };
};

