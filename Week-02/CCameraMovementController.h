#pragma once
#include <Windows.h>
#include "UCamera.h"

class CInputManager;

class CCameraMovementController
{
public:
	CCameraMovementController();
	~CCameraMovementController() {};

	void Initialize(HWND* InHWnd);
	void UpdateInputToCamera()
	{
		UpdateKeyboardInput();
		UpdateMouseDelta();
		ApplyToCamera();
	}

	HWND* hWnd = nullptr;
	POINT CurMouseDelta = { 0, 0 };
	CInputManager* Input; 

private:
	void UpdateKeyboardInput();
	void UpdateMouseDelta();
	void ApplyToCamera();

	//temp : gizmo 테스트용, 임시로 넣어둠
	void ApplyToGizmo();

	float ToRadian(float Degree);
	FVector Multiply(const FVector& vector, const FMatrix& matrix) const;

	UCamera& Camera;
	FVector KeyboardMoveDelta = { 0.0f, 0.0f, 0.0f };
	POINT PrevMousePos = { 0, 0 };
};

