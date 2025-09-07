#include "CameraInputMove.h"
#include "UCamera.h"

void CameraInputMove::Initialize(HWND* InHWnd)
{
	hWnd = InHWnd;
	PrevMousePos = { 0, 0 };
}

void CameraInputMove::UpdateMouseDelta()
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

void CameraInputMove::ApplyToCamera()
{
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // 오른쪽 마우스 버튼이 눌린 상태라면
	{
		UCamera::GetInstance().Rotation.Y += ToRadian((float)CurMouseDelta.x * 0.25f);
		UCamera::GetInstance().Rotation.X += ToRadian((float)CurMouseDelta.y * 0.25f);
		// X축 회전은 -89도 ~ +89도로 제한
		if (UCamera::GetInstance().Rotation.X > ToRadian(89.0f))
			UCamera::GetInstance().Rotation.X = ToRadian(89.0f);
		if (UCamera::GetInstance().Rotation.X < ToRadian(-89.0f))
			UCamera::GetInstance().Rotation.X = ToRadian(-89.0f);
	}
}

float CameraInputMove::ToRadian(float Degree)
{
	return Degree * Math::DegToRad;
}