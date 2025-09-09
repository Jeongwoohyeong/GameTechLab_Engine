#include "CCameraMovementController.h"
#include "CInputManager.h"
CCameraMovementController::CCameraMovementController() : Camera(UCamera::GetInstance()) {}

void CCameraMovementController::Initialize(HWND* InHWnd)
{
	hWnd = InHWnd;
	PrevMousePos = { 0, 0 };
	Input = &CInputManager::GetInstance();
}

void CCameraMovementController::UpdateKeyboardInput()
{
	KeyboardMoveDelta = { 0.0f, 0.0f, 0.0f };
	float moveSpeed = 0.05f;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		KeyboardMoveDelta.Z += moveSpeed;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		KeyboardMoveDelta.Z -= moveSpeed;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		KeyboardMoveDelta.X -= moveSpeed;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		KeyboardMoveDelta.X += moveSpeed;
	}
}

void CCameraMovementController::UpdateMouseDelta()
{
	POINT CurMousePos = { 0, 0 };

	// 현재 마우스 좌표 (화면 좌표)
	GetCursorPos(&CurMousePos);

	// 클라이언트 좌표로 변환 (윈도우 내부 좌표)
	ScreenToClient(*hWnd, &CurMousePos);

	// ΔX, ΔY 계산
	int deltaX = CurMousePos.x - PrevMousePos.x;
	int deltaY = CurMousePos.y - PrevMousePos.y;
	CurMouseDelta = { deltaX, deltaY };

	// 이번 프레임 좌표를 다음 프레임을 위해 저장
	PrevMousePos = CurMousePos;
}

void CCameraMovementController::ApplyToCamera()
{
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // 오른쪽 마우스 버튼이 눌린 상태라면
	{
		Camera.Rotation.Y += ToRadian((float)CurMouseDelta.x * 0.25f);
		Camera.Rotation.X += ToRadian((float)CurMouseDelta.y * 0.25f);
		// X축 회전은 -89도 ~ +89도로 제한
		if (Camera.Rotation.X > ToRadian(89.0f))
			Camera.Rotation.X = ToRadian(89.0f);
		if (Camera.Rotation.X < ToRadian(-89.0f))
			Camera.Rotation.X = ToRadian(-89.0f);
	}

	Camera.Location += Multiply(KeyboardMoveDelta, FMatrix::MakeRotation(Camera.GetInstance().Rotation));
}



void CCameraMovementController::ApplyToGizmo()
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) // 오른쪽 마우스 버튼이 눌린 상태라면
	{
		
	}
}

float CCameraMovementController::ToRadian(float Degree)
{
	return Degree * Math::DegToRad;
}

FVector CCameraMovementController::Multiply(const FVector& v, const FMatrix& M) const
{
	return {
		v.X * M.M[0][0] + v.Y * M.M[1][0] + v.Z * M.M[2][0],
		v.X * M.M[0][1] + v.Y * M.M[1][1] + v.Z * M.M[2][1],
		v.X * M.M[0][2] + v.Y * M.M[1][2] + v.Z * M.M[2][2]
	};
}